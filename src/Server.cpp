/**
 * \file Server.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

/**
  Docu   : To use the lyxserver define the name of the pipe in your
	   lyxrc:
	   \serverpipe "/home/myhome/.lyxpipe"
	   Then use .lyxpipe.in and .lyxpipe.out to communicate to LyX.
	   Each message consists of a single line in ASCII. Input lines
	   (client -> LyX) have the following format:
	    "LYXCMD:<clientname>:<functionname>:<argument>"
	   Answers from LyX look like this:
	   "INFO:<clientname>:<functionname>:<data>"
 [asierra970531] Or like this in case of error:
	   "ERROR:<clientname>:<functionname>:<error message>"
	   where <clientname> and <functionname> are just echoed.
	   If LyX notifies about a user defined extension key-sequence,
	   the line looks like this:
	   "NOTIFY:<key-sequence>"
 [asierra970531] New server-only messages to implement a simple protocol
	   "LYXSRV:<clientname>:<protocol message>"
	   where <protocol message> can be "hello" or "bye". If hello is
	   received LyX will inform the client that it's listening its
	   messages, and 'bye' will inform that lyx is closing.

	   See development/lyxserver/server_monitor.cpp for an example client.
  Purpose: implement a client/server lib for LyX
*/

#include <config.h>

#include "Server.h"

#include "DispatchResult.h"
#include "FuncRequest.h"
#include "LyX.h"
#include "LyXAction.h"

#include "frontends/Application.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/os.h"

#include "support/bind.h"

#ifdef _WIN32
#include <io.h>
#include <QCoreApplication>
#endif
#include <QThread>

#include <cerrno>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <fcntl.h>

using namespace std;
using namespace lyx::support;
using os::external_path;

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// LyXComm
//
/////////////////////////////////////////////////////////////////////

#if defined(_WIN32)

class ReadReadyEvent : public QEvent {
public:
	///
	ReadReadyEvent(DWORD inpipe) : QEvent(QEvent::User), inpipe_(inpipe)
	{}
	///
	DWORD inpipe() const { return inpipe_; }

private:
	DWORD inpipe_;
};

namespace {

string errormsg(DWORD const error)
{
	void * msgbuf;
	string message;
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			  FORMAT_MESSAGE_FROM_SYSTEM |
			  FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL, error,
			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			  (LPTSTR) &msgbuf, 0, NULL)) {
		message = static_cast<char *>(msgbuf);
		LocalFree(msgbuf);
	} else
		message = "Unknown error";

	return message;
}

} // namespace anon


DWORD WINAPI pipeServerWrapper(void * arg)
{
	LyXComm * lyxcomm = reinterpret_cast<LyXComm *>(arg);
	if (!lyxcomm->pipeServer()) {
		// Error exit; perform cleanup.
		lyxcomm->ready_ = false;
		lyxcomm->closeHandles();
		CloseHandle(lyxcomm->server_thread_);
		CloseHandle(lyxcomm->stopserver_);
		CloseHandle(lyxcomm->outbuf_mutex_);
		lyxerr << "LyXComm: Closing connection" << endl;
	}
	return 1;
}


LyXComm::LyXComm(string const & pip, Server * cli, ClientCallbackfct ccb)
	: pipename_(pip), client_(cli), clientcb_(ccb), stopserver_(0)
{
	for (int i = 0; i < MAX_PIPES; ++i) {
		event_[i] = 0;
		pipe_[i].handle = INVALID_HANDLE_VALUE;
	}
	ready_ = false;
	deferred_loading_ = false;
	openConnection();
}


bool LyXComm::pipeServer()
{
	DWORD i;
	DWORD error;

	for (i = 0; i < MAX_PIPES; ++i) {
		bool const is_outpipe = i >= MAX_CLIENTS;
		DWORD const open_mode = is_outpipe ? PIPE_ACCESS_OUTBOUND
						   : PIPE_ACCESS_INBOUND;
		string const pipename = external_path(pipeName(i));

		// Manual-reset event, initial state = signaled
		event_[i] = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (!event_[i]) {
			error = GetLastError();
			lyxerr << "LyXComm: Could not create event for pipe "
			       << pipename << "\nLyXComm: "
			       << errormsg(error) << endl;
			return false;
		}

		pipe_[i].overlap.hEvent = event_[i];
		pipe_[i].iobuf.erase();
		pipe_[i].handle = CreateNamedPipe(pipename.c_str(),
				open_mode | FILE_FLAG_OVERLAPPED, PIPE_WAIT,
				MAX_CLIENTS, PIPE_BUFSIZE, PIPE_BUFSIZE,
				PIPE_TIMEOUT, NULL);

		if (pipe_[i].handle == INVALID_HANDLE_VALUE) {
			error = GetLastError();
			lyxerr << "LyXComm: Could not create pipe "
			       << pipename << "\nLyXComm: "
			       << errormsg(error) << endl;
			return false;
		}

		if (!startPipe(i))
			return false;
		pipe_[i].state = pipe_[i].pending_io ?
			CONNECTING_STATE : (is_outpipe ? WRITING_STATE
						       : READING_STATE);
	}

	// Add the stopserver_ event
	event_[MAX_PIPES] = stopserver_;

	// We made it!
	LYXERR(Debug::LYXSERVER, "LyXComm: Connection established");
	ready_ = true;
	outbuf_.erase();
	DWORD status;
	bool success;

	while (!checkStopServer()) {
		// Indefinitely wait for the completion of an overlapped
		// read, write, or connect operation.
		DWORD wait = WaitForMultipleObjects(MAX_PIPES + 1, event_,
						    FALSE, INFINITE);

		// Determine which pipe instance completed the operation.
		i = wait - WAIT_OBJECT_0;
		LASSERT(i >= 0 && i <= MAX_PIPES, /**/);

		// Check whether we were waked up for stopping the pipe server.
		if (i == MAX_PIPES)
			break;

		bool const is_outpipe = i >= MAX_CLIENTS;

		// Get the result if the operation was pending.
		if (pipe_[i].pending_io) {
			success = GetOverlappedResult(pipe_[i].handle,
					&pipe_[i].overlap, &status, FALSE);

			switch (pipe_[i].state) {
			case CONNECTING_STATE:
				// Pending connect operation
				if (!success) {
					error = GetLastError();
					lyxerr << "LyXComm: "
					       << errormsg(error) << endl;
					if (!resetPipe(i, true))
						return false;
					continue;
				}
				pipe_[i].state = is_outpipe ? WRITING_STATE
							    : READING_STATE;
				break;

			case READING_STATE:
				// Pending read operation
				LASSERT(!is_outpipe, /**/);
				if (!success || status == 0) {
					if (!resetPipe(i, !success))
						return false;
					continue;
				}
				pipe_[i].nbytes = status;
				pipe_[i].state = WRITING_STATE;
				break;

			case WRITING_STATE:
				// Pending write operation
				LASSERT(is_outpipe, /**/);
				// Let's see whether we have a reply
				if (!outbuf_.empty()) {
					// Yep. Deliver it to all pipe
					// instances if we get ownership
					// of the mutex, otherwise we'll
					// try again the next round.
					DWORD result = WaitForSingleObject(
							outbuf_mutex_, 200);
					if (result == WAIT_OBJECT_0) {
						DWORD j = MAX_CLIENTS;
						while (j < MAX_PIPES) {
							pipe_[j].iobuf = outbuf_;
							++j;
						}
						outbuf_.erase();
					}
					ReleaseMutex(outbuf_mutex_);
				}
				if (pipe_[i].iobuf.empty())
					pipe_[i].pending_io = false;
				break;
			}
		}

		// Operate according to the pipe state
		switch (pipe_[i].state) {
		case READING_STATE:
			// The pipe instance is connected to a client
			// and is ready to read a request.
			LASSERT(!is_outpipe, /**/);
			success = ReadFile(pipe_[i].handle,
					pipe_[i].readbuf, PIPE_BUFSIZE - 1,
					&pipe_[i].nbytes, &pipe_[i].overlap);

			if (success && pipe_[i].nbytes != 0) {
				// The read operation completed successfully.
				pipe_[i].pending_io = false;
				pipe_[i].state = WRITING_STATE;
				continue;
			}

			error = GetLastError();

			if (!success && error == ERROR_IO_PENDING) {
				// The read operation is still pending.
				pipe_[i].pending_io = true;
				continue;
			}

			success = error == ERROR_BROKEN_PIPE;

			// Client closed connection (ERROR_BROKEN_PIPE) or
			// an error occurred; in either case, reset the pipe.
			if (!success) {
				lyxerr << "LyXComm: " << errormsg(error) << endl;
				if (!pipe_[i].iobuf.empty()) {
					lyxerr << "LyXComm: truncated command: "
					       << pipe_[i].iobuf << endl;
					pipe_[i].iobuf.erase();
				}
			}
			if (!resetPipe(i, !success))
				return false;
			break;

		case WRITING_STATE:
			if (!is_outpipe) {
				// The request was successfully read
				// from the client; commit it.
				ReadReadyEvent * event = new ReadReadyEvent(i);
				QCoreApplication::postEvent(this,
						static_cast<QEvent *>(event));
				// Wait for completion
				while (pipe_[i].nbytes && !checkStopServer(100))
					;
				pipe_[i].pending_io = false;
				pipe_[i].state = READING_STATE;
				continue;
			}

			// This is an output pipe instance. Initiate the
			// overlapped write operation or monitor its progress.

			if (pipe_[i].pending_io) {
				success = WriteFile(pipe_[i].handle,
						pipe_[i].iobuf.c_str(),
						pipe_[i].iobuf.length(),
						&status,
						&pipe_[i].overlap);
			}

			if (success && !pipe_[i].iobuf.empty()
			    && status == pipe_[i].iobuf.length()) {
				// The write operation completed successfully.
				pipe_[i].iobuf.erase();
				pipe_[i].pending_io = false;
				if (!resetPipe(i))
					return false;
				continue;
			}

			error = GetLastError();

			if (success && error == ERROR_IO_PENDING) {
				// The write operation is still pending.
				// We get here when a reader is started
				// well before a reply is ready, so delay
				// a bit in order to not burden the cpu.
				checkStopServer(100);
				pipe_[i].pending_io = true;
				continue;
			}

			success = error == ERROR_NO_DATA;

			// Client closed connection (ERROR_NO_DATA) or
			// an error occurred; in either case, reset the pipe.
			if (!success) {
				lyxerr << "LyXComm: Error sending message: "
				       << pipe_[i].iobuf << "\nLyXComm: "
				       << errormsg(error) << endl;
			}
			if (!resetPipe(i, !success))
				return false;
			break;
		}
	}

	ready_ = false;
	closeHandles();
	return true;
}


void LyXComm::closeHandles()
{
	for (int i = 0; i < MAX_PIPES; ++i) {
		if (event_[i]) {
			ResetEvent(event_[i]);
			CloseHandle(event_[i]);
			event_[i] = 0;
		}
		if (pipe_[i].handle != INVALID_HANDLE_VALUE) {
			CloseHandle(pipe_[i].handle);
			pipe_[i].handle = INVALID_HANDLE_VALUE;
		}
	}
}


bool LyXComm::event(QEvent * e)
{
	if (e->type() == QEvent::User) {
		read_ready(static_cast<ReadReadyEvent *>(e)->inpipe());
		return true;
	}
	return false;
}


bool LyXComm::checkStopServer(DWORD timeout)
{
	return WaitForSingleObject(stopserver_, timeout) == WAIT_OBJECT_0;
}


bool LyXComm::startPipe(DWORD index)
{
	pipe_[index].pending_io = false;
	pipe_[index].overlap.Offset = 0;
	pipe_[index].overlap.OffsetHigh = 0;

	// Overlapped ConnectNamedPipe should return zero.
	if (ConnectNamedPipe(pipe_[index].handle, &pipe_[index].overlap)) {
		DWORD const error = GetLastError();
		lyxerr << "LyXComm: Could not connect pipe "
		       << external_path(pipeName(index))
		       << "\nLyXComm: " << errormsg(error) << endl;
		return false;
	}

	switch (GetLastError()) {
	case ERROR_IO_PENDING:
		// The overlapped connection is in progress.
		pipe_[index].pending_io = true;
		break;

	case ERROR_PIPE_CONNECTED:
		// Client is already connected, so signal an event.
		if (SetEvent(pipe_[index].overlap.hEvent))
			break;
		// fall through
	default:
		// Anything else is an error.
		DWORD const error = GetLastError();
		lyxerr << "LyXComm: An error occurred while connecting pipe "
		       << external_path(pipeName(index))
		       << "\nLyXComm: " << errormsg(error) << endl;
		return false;
	}

	return true;
}


bool LyXComm::resetPipe(DWORD index, bool close_handle)
{
	// This method is called when an error occurs or when a client
	// closes the connection. We first disconnect the pipe instance,
	// then reconnect it, ready to wait for another client.

	if (!DisconnectNamedPipe(pipe_[index].handle)) {
		DWORD const error = GetLastError();
		lyxerr << "LyXComm: Could not disconnect pipe "
		       << external_path(pipeName(index))
		       << "\nLyXComm: " << errormsg(error) << endl;
		// What to do now? Let's try whether re-creating the pipe helps.
		close_handle = true;
	}

	bool const is_outpipe = index >= MAX_CLIENTS;

	if (close_handle) {
		DWORD const open_mode = is_outpipe ? PIPE_ACCESS_OUTBOUND
						   : PIPE_ACCESS_INBOUND;
		string const name = external_path(pipeName(index));

		CloseHandle(pipe_[index].handle);

		pipe_[index].iobuf.erase();
		pipe_[index].handle = CreateNamedPipe(name.c_str(),
				open_mode | FILE_FLAG_OVERLAPPED, PIPE_WAIT,
				MAX_CLIENTS, PIPE_BUFSIZE, PIPE_BUFSIZE,
				PIPE_TIMEOUT, NULL);

		if (pipe_[index].handle == INVALID_HANDLE_VALUE) {
			DWORD const error = GetLastError();
			lyxerr << "LyXComm: Could not reset pipe " << name
			       << "\nLyXComm: " << errormsg(error) << endl;
			return false;
		}
	}

	if (!startPipe(index))
		return false;
	pipe_[index].state = pipe_[index].pending_io ?
			CONNECTING_STATE : (is_outpipe ? WRITING_STATE
						       : READING_STATE);
	return true;
}


void LyXComm::openConnection()
{
	LYXERR(Debug::LYXSERVER, "LyXComm: Opening connection");

	// If we are up, that's an error
	if (ready_) {
		LYXERR(Debug::LYXSERVER, "LyXComm: Already connected");
		return;
	}

	if (pipename_.empty()) {
		LYXERR(Debug::LYXSERVER, "LyXComm: server is disabled, nothing to do");
		return;
	}

	// Check whether the pipe name is being used by some other instance.
	if (!stopserver_ && WaitNamedPipe(inPipeName().c_str(), 0)) {
		// Tell the running instance to load the files
		if (run_mode == USE_REMOTE && loadFilesInOtherInstance()) {
			deferred_loading_ = true;
			pipename_.erase();
			return;
		}
		lyxerr << "LyXComm: Pipe " << external_path(inPipeName())
		       << " already exists.\nMaybe another instance of LyX"
			  " is using it." << endl;
		pipename_.erase();
		return;
	}

	// Mutex with no initial owner for synchronized access to outbuf_
	outbuf_mutex_ = CreateMutex(NULL, FALSE, NULL);
	if (!outbuf_mutex_) {
		DWORD const error = GetLastError();
		lyxerr << "LyXComm: Could not create output buffer mutex"
		       << "\nLyXComm: " << errormsg(error) << endl;
		pipename_.erase();
		return;
	}

	// Manual-reset event, initial state = not signaled
	stopserver_ = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!stopserver_) {
		DWORD const error = GetLastError();
		lyxerr << "LyXComm: Could not create stop server event"
		       << "\nLyXComm: " << errormsg(error) << endl;
		pipename_.erase();
		CloseHandle(outbuf_mutex_);
		return;
	}

	server_thread_ = CreateThread(NULL, 0, pipeServerWrapper,
				     static_cast<void *>(this), 0, NULL);
	if (!server_thread_) {
		DWORD const error = GetLastError();
		lyxerr << "LyXComm: Could not create pipe server thread"
		       << "\nLyXComm: " << errormsg(error) << endl;
		pipename_.erase();
		CloseHandle(stopserver_);
		CloseHandle(outbuf_mutex_);
		return;
	}
}


/// Close pipes
void LyXComm::closeConnection()
{
	LYXERR(Debug::LYXSERVER, "LyXComm: Closing connection");

	if (pipename_.empty()) {
		LYXERR(Debug::LYXSERVER, "LyXComm: server is disabled, nothing to do");
		return;
	}

	if (!ready_) {
		LYXERR(Debug::LYXSERVER, "LyXComm: Already disconnected");
		return;
	}

	SetEvent(stopserver_);
	// Wait for the pipe server to finish
	WaitForSingleObject(server_thread_, INFINITE);
	CloseHandle(server_thread_);
	ResetEvent(stopserver_);
	CloseHandle(stopserver_);
	CloseHandle(outbuf_mutex_);
}


void LyXComm::emergencyCleanup()
{
	if (ready_) {
		SetEvent(stopserver_);
		// Forcibly terminate the pipe server thread if it does
		// not finish quickly.
		if (WaitForSingleObject(server_thread_, 200) != WAIT_OBJECT_0) {
			TerminateThread(server_thread_, 0);
			ready_ = false;
			closeHandles();
		}
		CloseHandle(server_thread_);
		ResetEvent(stopserver_);
		CloseHandle(stopserver_);
		CloseHandle(outbuf_mutex_);
	}
}


void LyXComm::read_ready(DWORD inpipe)
{
	// Turn the pipe buffer into a C string
	DWORD const nbytes = pipe_[inpipe].nbytes;
	pipe_[inpipe].readbuf[nbytes] = '\0';

	pipe_[inpipe].iobuf += rtrim(pipe_[inpipe].readbuf, "\r");

	// Commit any commands read
	while (pipe_[inpipe].iobuf.find('\n') != string::npos) {
		// split() grabs the entire string if
		// the delim /wasn't/ found. ?:-P
		string cmd;
		pipe_[inpipe].iobuf = split(pipe_[inpipe].iobuf, cmd, '\n');
		cmd = rtrim(cmd, "\r");
		LYXERR(Debug::LYXSERVER, "LyXComm: nbytes:" << nbytes
			<< ", iobuf:" << pipe_[inpipe].iobuf
			<< ", cmd:" << cmd);
		if (!cmd.empty())
			clientcb_(client_, cmd);
			//\n or not \n?
	}
	// Signal that we are done.
	pipe_[inpipe].nbytes = 0;
}


void LyXComm::send(string const & msg)
{
	if (msg.empty()) {
		lyxerr << "LyXComm: Request to send empty string. Ignoring."
		       << endl;
		return;
	}

	LYXERR(Debug::LYXSERVER, "LyXComm: Sending '" << msg << '\'');

	if (pipename_.empty())
		return;

	if (!ready_) {
		lyxerr << "LyXComm: Pipes are closed. Could not send "
		       << msg << endl;
		return;
	}

	// Request ownership of the outbuf_mutex_
	DWORD result = WaitForSingleObject(outbuf_mutex_, PIPE_TIMEOUT);

	if (result == WAIT_OBJECT_0) {
		// If a client doesn't care to read a reply (JabRef is one
		// such client), the output buffer could grow without limit.
		// So, we empty it when its size is larger than PIPE_BUFSIZE.
		if (outbuf_.size() > PIPE_BUFSIZE)
			outbuf_.erase();
		outbuf_ += msg;
		ReleaseMutex(outbuf_mutex_);
	} else {
		// Something is fishy, better resetting the connection.
		DWORD const error = GetLastError();
		lyxerr << "LyXComm: Error sending message: " << msg
		       << "\nLyXComm: " << errormsg(error)
		       << "\nLyXComm: Resetting connection" << endl;
		ReleaseMutex(outbuf_mutex_);
		closeConnection();
		openConnection();
	}
}


string const LyXComm::pipeName(DWORD index) const
{
	return index < MAX_CLIENTS ? inPipeName() : outPipeName();
}


#elif !defined (HAVE_MKFIFO)
// We provide a stub class that disables the lyxserver.

LyXComm::LyXComm(string const &, Server *, ClientCallbackfct)
{}


void LyXComm::openConnection()
{}


void LyXComm::closeConnection()
{}


int LyXComm::startPipe(string const & filename, bool write)
{
	return -1;
}


void LyXComm::endPipe(int & fd, string const & filename, bool write)
{}


void LyXComm::emergencyCleanup()
{}


void LyXComm::read_ready()
{}


void LyXComm::send(string const & msg)
{}


#else // defined (HAVE_MKFIFO)

LyXComm::LyXComm(string const & pip, Server * cli, ClientCallbackfct ccb)
	: pipename_(pip), client_(cli), clientcb_(ccb)
{
	ready_ = false;
	deferred_loading_ = false;
	openConnection();
}


void LyXComm::openConnection()
{
	LYXERR(Debug::LYXSERVER, "LyXComm: Opening connection");

	// If we are up, that's an error
	if (ready_) {
		lyxerr << "LyXComm: Already connected" << endl;
		return;
	}
	// We assume that we don't make it
	ready_ = false;

	if (pipename_.empty()) {
		LYXERR(Debug::LYXSERVER, "LyXComm: server is disabled, nothing to do");
		return;
	}

	infd_ = startPipe(inPipeName(), false);
	if (infd_ == -1)
		return;

	outfd_ = startPipe(outPipeName(), true);
	if (outfd_ == -1) {
		endPipe(infd_, inPipeName(), false);
		return;
	}

	if (fcntl(outfd_, F_SETFL, O_NONBLOCK) < 0) {
		lyxerr << "LyXComm: Could not set flags on pipe " << outPipeName()
		       << '\n' << strerror(errno) << endl;
		return;
	}

	// We made it!
	ready_ = true;
	LYXERR(Debug::LYXSERVER, "LyXComm: Connection established");
}


/// Close pipes
void LyXComm::closeConnection()
{
	LYXERR(Debug::LYXSERVER, "LyXComm: Closing connection");

	if (pipename_.empty()) {
		LYXERR(Debug::LYXSERVER, "LyXComm: server is disabled, nothing to do");
		return;
	}

	if (!ready_) {
		LYXERR0("LyXComm: Already disconnected");
		return;
	}

	endPipe(infd_, inPipeName(), false);
	endPipe(outfd_, outPipeName(), true);

	ready_ = false;
}


int LyXComm::startPipe(string const & file, bool write)
{
	static bool stalepipe = false;
	FileName const filename(file);
	if (filename.exists()) {
		if (!write) {
			// Let's see whether we have a stale pipe.
			int fd = ::open(filename.toFilesystemEncoding().c_str(),
					O_WRONLY | O_NONBLOCK);
			if (fd >= 0) {
				// Another LyX instance is using it.
				::close(fd);
				// Tell the running instance to load the files
				if (run_mode == USE_REMOTE && loadFilesInOtherInstance()) {
					deferred_loading_ = true;
					pipename_.erase();
					return -1;
				}
			} else if (errno == ENXIO) {
				// No process is reading from the other end.
				stalepipe = true;
				LYXERR(Debug::LYXSERVER,
					"LyXComm: trying to remove "
					<< filename);
				filename.removeFile();
			}
		} else if (stalepipe) {
			LYXERR(Debug::LYXSERVER, "LyXComm: trying to remove "
				<< filename);
			filename.removeFile();
			stalepipe = false;
		}
		if (filename.exists()) {
			lyxerr << "LyXComm: Pipe " << filename
			       << " already exists.\nIf no other LyX program"
			          " is active, please delete the pipe by hand"
				  " and try again."
			       << endl;
			pipename_.erase();
			return -1;
		}
	}

	if (::mkfifo(filename.toFilesystemEncoding().c_str(), 0600) < 0) {
		lyxerr << "LyXComm: Could not create pipe " << filename << '\n'
		       << strerror(errno) << endl;
		return -1;
	}
	int const fd = ::open(filename.toFilesystemEncoding().c_str(),
			      write ? (O_RDWR) : (O_RDONLY|O_NONBLOCK));

	if (fd < 0) {
		lyxerr << "LyXComm: Could not open pipe " << filename << '\n'
		       << strerror(errno) << endl;
		filename.removeFile();
		return -1;
	}

	if (!write) {
		theApp()->registerSocketCallback(fd,
			bind(&LyXComm::read_ready, this));
	}

	return fd;
}


void LyXComm::endPipe(int & fd, string const & filename, bool write)
{
	if (fd < 0)
		return;

	if (!write)
		theApp()->unregisterSocketCallback(fd);

	if (::close(fd) < 0) {
		lyxerr << "LyXComm: Could not close pipe " << filename
		       << '\n' << strerror(errno) << endl;
	}

	if (!FileName(filename).removeFile()) {
		lyxerr << "LyXComm: Could not remove pipe " << filename
		       << '\n' << strerror(errno) << endl;
	}

	fd = -1;
}


void LyXComm::emergencyCleanup()
{
	if (!pipename_.empty()) {
		endPipe(infd_, inPipeName(), false);
		endPipe(outfd_, outPipeName(), true);
	}
}


// Receives messages and sends then to client
void LyXComm::read_ready()
{
	// FIXME: make read_buffer_ a class-member for multiple sessions
	static string read_buffer_;
	read_buffer_.erase();

	int const charbuf_size = 100;
	char charbuf[charbuf_size];

	// As O_NONBLOCK is set, until no data is available for reading,
	// read() doesn't block but returns -1 and set errno to EAGAIN.
	// After a client that opened the pipe for writing, closes it
	// (and no other client is using the pipe), read() would always
	// return 0 and thus the connection has to be reset.

	errno = 0;
	int status;
	// the single = is intended here.
	while ((status = ::read(infd_, charbuf, charbuf_size - 1))) {
		if (status > 0) {
			charbuf[status] = '\0'; // turn it into a c string
			read_buffer_ += rtrim(charbuf, "\r");
			// commit any commands read
			while (read_buffer_.find('\n') != string::npos) {
				// split() grabs the entire string if
				// the delim /wasn't/ found. ?:-P
				string cmd;
				read_buffer_= split(read_buffer_, cmd,'\n');
				LYXERR(Debug::LYXSERVER, "LyXComm: status:" << status
					<< ", read_buffer_:" << read_buffer_
					<< ", cmd:" << cmd);
				if (!cmd.empty())
					clientcb_(client_, cmd);
					//\n or not \n?
			}
		} else {
			if (errno == EAGAIN) {
				// Nothing to read, continue
				errno = 0;
				return;
			}
			// An error occurred, better bailing out
			LYXERR0("LyXComm: " << strerror(errno));
			if (!read_buffer_.empty()) {
				LYXERR0("LyXComm: truncated command: " << read_buffer_);
				read_buffer_.erase();
			}
			break; // reset connection
		}
	}

	// The connection gets reset when read() returns 0 (meaning that the
	// last client closed the pipe) or an error occurred, in which case
	// read() returns -1 and errno != EAGAIN.
	closeConnection();
	openConnection();
	errno = 0;
}


void LyXComm::send(string const & msg)
{
	if (msg.empty()) {
		LYXERR0("LyXComm: Request to send empty string. Ignoring.");
		return;
	}

	LYXERR(Debug::LYXSERVER, "LyXComm: Sending '" << msg << '\'');

	if (pipename_.empty())
		return;

	if (!ready_) {
		LYXERR0("LyXComm: Pipes are closed. Could not send " << msg);
	} else if (::write(outfd_, msg.c_str(), msg.length()) < 0) {
		lyxerr << "LyXComm: Error sending message: " << msg
		       << '\n' << strerror(errno)
		       << "\nLyXComm: Resetting connection" << endl;
		closeConnection();
		openConnection();
	}
}

#endif // defined (HAVE_MKFIFO)

namespace {

struct Sleep : QThread
{
	static void millisec(unsigned long ms)
	{
		QThread::usleep(ms * 1000);
	}
};

} // namespace anon


bool LyXComm::loadFilesInOtherInstance()
{
	int pipefd;
	int loaded_files = 0;
	FileName const pipe(inPipeName());
	vector<string>::iterator it = theFilesToLoad().begin();
	while (it != theFilesToLoad().end()) {
		FileName fname = fileSearch(string(), os::internal_path(*it),
						"lyx", may_not_exist);
		if (fname.empty()) {
			++it;
			continue;
		}
		// Wait a while to allow time for the other
		// instance to reset the connection
		Sleep::millisec(200);
		pipefd = ::open(pipe.toFilesystemEncoding().c_str(), O_WRONLY);
		if (pipefd < 0)
			break;
		string const cmd = "LYXCMD:pipe:file-open:" +
					fname.absFileName() + '\n';
		if (::write(pipefd, cmd.c_str(), cmd.length()) < 0)
			LYXERR0("Cannot write to pipe!");
		::close(pipefd);
		++loaded_files;
		it = theFilesToLoad().erase(it);
	}
	return loaded_files > 0;
}


string const LyXComm::inPipeName() const
{
	return pipename_ + ".in";
}


string const LyXComm::outPipeName() const
{
	return pipename_ + ".out";
}


/////////////////////////////////////////////////////////////////////
//
// Server
//
/////////////////////////////////////////////////////////////////////

void ServerCallback(Server * server, string const & msg)
{
	server->callback(msg);
}

Server::Server(string const & pipes)
	: numclients_(0), pipes_(pipes, this, &ServerCallback)
{}


Server::~Server()
{
	// say goodbye to clients so they stop sending messages
	// send as many bye messages as there are clients,
	// each with client's name.
	string message;
	for (int i = 0; i != numclients_; ++i) {
		message = "LYXSRV:" + clients_[i] + ":bye\n";
		pipes_.send(message);
	}
}


int compare(char const * a, char const * b, unsigned int len)
{
	return strncmp(a, b, len);
}


// Handle data gotten from communication, called by LyXComm
void Server::callback(string const & msg)
{
	LYXERR(Debug::LYXSERVER, "Server: Received: '" << msg << '\'');

	char const * p = msg.c_str();

	// --- parse the string --------------------------------------------
	//
	//  Format: LYXCMD:<client>:<func>:<argstring>\n
	//
	bool server_only = false;
	while (*p) {
		// --- 1. check 'header' ---

		if (compare(p, "LYXSRV:", 7) == 0) {
			server_only = true;
		} else if (0 != compare(p, "LYXCMD:", 7)) {
			lyxerr << "Server: Unknown request \""
			       << p << '"' << endl;
			return;
		}
		p += 7;

		// --- 2. for the moment ignore the client name ---
		string client;
		while (*p && *p != ':')
			client += char(*p++);
		if (*p == ':')
			++p;
		if (!*p)
			return;

		// --- 3. get function name ---
		string cmd;
		while (*p && *p != ':')
			cmd += char(*p++);

		// --- 4. parse the argument ---
		string arg;
		if (!server_only && *p == ':' && *(++p)) {
			while (*p && *p != '\n')
				arg += char(*p++);
			if (*p) ++p;
		}

		LYXERR(Debug::LYXSERVER, "Server: Client: '" << client
			<< "' Command: '" << cmd << "' Argument: '" << arg << '\'');

		// --- lookup and exec the command ------------------

		if (server_only) {
			string buf;
			// return the greeting to inform the client that
			// we are listening.
			if (cmd == "hello") {
				// One more client
				if (numclients_ == MAX_CLIENTS) { //paranoid check
					LYXERR(Debug::LYXSERVER, "Server: too many clients...");
					return;
				}
				int i = 0;
				while (!clients_[i].empty() && i < numclients_)
					++i;
				clients_[i] = client;
				++numclients_;
				buf = "LYXSRV:" + client + ":hello\n";
				LYXERR(Debug::LYXSERVER, "Server: Greeting " << client);
				pipes_.send(buf);
			} else if (cmd == "bye") {
				// If clients_ == 0 maybe we should reset the pipes
				// to prevent fake callbacks
				int i = 0; //look if client is registered
				for (; i < numclients_; ++i) {
					if (clients_[i] == client)
						break;
				}
				if (i < numclients_) {
					--numclients_;
					clients_[i].erase();
					LYXERR(Debug::LYXSERVER, "Server: Client "
						<< client << " said goodbye");
				} else {
					LYXERR(Debug::LYXSERVER,
						"Server: ignoring bye messge from unregistered client" << client);
				}
			} else {
				LYXERR0("Server: Undefined server command " << cmd << '.');
			}
			return;
		}

		if (!cmd.empty()) {
			// which lyxfunc should we let it connect to?
			// The correct solution would be to have a
			// specialized (non-gui) BufferView. But how do
			// we do it now? Probably we should just let it
			// connect to the lyxfunc in the single GuiView we
			// support currently. (Lgb)

			FuncRequest fr(lyxaction.lookupFunc(cmd), arg);
			fr.setOrigin(FuncRequest::LYXSERVER);
			DispatchResult dr;
			theApp()->dispatch(fr, dr);
			string const rval = to_utf8(dr.message());

			// all commands produce an INFO or ERROR message
			// in the output pipe, even if they do not return
			// anything. See chapter 4 of Customization doc.
			string buf;
			if (dr.error())
				buf = "ERROR:";
			else
				buf = "INFO:";
			buf += client + ':' + cmd + ':' +  rval + '\n';
			pipes_.send(buf);

			// !!! we don't do any error checking -
			//  if the client won't listen, the
			//  message is lost and others too
			//  maybe; so the client should empty
			//  the outpipe before issuing a request.

			// not found
		}
	}  // while *p
}


// Send a notify message to a client, called by WorkAreaKeyPress
void Server::notifyClient(string const & s)
{
	pipes_.send("NOTIFY:" + s + "\n");
}


} // namespace lyx

#ifdef _WIN32
#include "moc_Server.cpp"
#endif
