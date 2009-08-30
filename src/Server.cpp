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

	   See development/server_monitor.c for an example client.
  Purpose: implement a client/server lib for LyX
*/

#include <config.h>

#include "Server.h"
#include "FuncRequest.h"
#include "LyXAction.h"
#include "LyXFunc.h"

#include "frontends/Application.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/os.h"

#include <boost/bind.hpp>

#ifdef _WIN32
#include <QCoreApplication>
#endif

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

char * errormsg()
{
	void * msgbuf;
	DWORD error = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		      FORMAT_MESSAGE_FROM_SYSTEM |
		      FORMAT_MESSAGE_IGNORE_INSERTS,
		      NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		      (LPTSTR) &msgbuf, 0, NULL);
	return static_cast<char *>(msgbuf);
}


extern "C" {

DWORD WINAPI pipeServerWrapper(void * arg)
{
	LyXComm * lyxcomm = reinterpret_cast<LyXComm *>(arg);
	lyxcomm->pipeServer();
	return 1;
}

}

} // namespace anon

LyXComm::LyXComm(string const & pip, Server * cli, ClientCallbackfct ccb)
	: pipename_(pip), client_(cli), clientcb_(ccb), stopserver_(0)
{
	ready_ = false;
	openConnection();
}


void LyXComm::pipeServer()
{
	DWORD i;

	for (i = 0; i <= MAX_PIPES; ++i) {
		DWORD open_mode;
		string const pipename = external_path(pipeName(i));

		if (i < MAX_PIPES) {
			open_mode = PIPE_ACCESS_INBOUND;
			readbuf_[i].erase();
		} else {
			open_mode = PIPE_ACCESS_OUTBOUND;
			writebuf_.erase();
		}

		// Manual-reset event, initial state = signaled
		event_[i] = CreateEvent(NULL, TRUE, TRUE, NULL);

		if (!event_[i]) {
			LYXERR0("LyXComm: Could not create event for pipe "
				<< pipename.c_str() << '\n' << errormsg());
			closeHandles(i);
			return;
		}

		pipe_[i].overlap.hEvent = event_[i];
		pipe_[i].handle = CreateNamedPipe(pipename.c_str(),
				open_mode | FILE_FLAG_OVERLAPPED, PIPE_WAIT,
				MAX_PIPES, PIPE_BUFSIZE, PIPE_BUFSIZE,
				PIPE_TIMEOUT, NULL);

		if (pipe_[i].handle == INVALID_HANDLE_VALUE) {
			LYXERR0("LyXComm: Could not create pipe "
				<< pipename.c_str() << '\n' << errormsg());
			closeHandles(i);
			return;
		}

		startPipe(i);
		pipe_[i].state = pipe_[i].pending_io ?
			CONNECTING_STATE : (i < MAX_PIPES ? READING_STATE
							  : WRITING_STATE);
	}

	// Add the stopserver_ event
	event_[MAX_PIPES + 1] = stopserver_;

	// We made it!
	LYXERR(Debug::LYXSERVER, "LyXComm: Connection established");
	ready_ = true;
	DWORD status;
	bool success;

	while (!checkStopServer()) {
		// Indefinitely wait for the completion of an overlapped
		// read, write, or connect operation.
		DWORD wait = WaitForMultipleObjects(MAX_PIPES + 2, event_,
						    FALSE, INFINITE);

		// Determine which pipe instance completed the operation.
		i = wait - WAIT_OBJECT_0;
		LASSERT(i >= 0 && i <= MAX_PIPES + 1, /**/);

		// Check whether we were waked up for stopping the pipe server.
		if (i == MAX_PIPES + 1)
			break;

		// Get the result if the operation was pending.
		if (pipe_[i].pending_io) {
			success = GetOverlappedResult(pipe_[i].handle,
					&pipe_[i].overlap, &status, FALSE);

			switch (pipe_[i].state) {
			case CONNECTING_STATE:
				// Pending connect operation
				if (!success) {
					DWORD const err = GetLastError();
					if (i == MAX_PIPES
					    && err == ERROR_IO_INCOMPLETE) {
						// A reply on the output pipe
						// has not been read, still.
						// As we have only one instance
						// for output, we risk a stalled
						// pipe if no one reads it.
						// So, if a reader doesn't
						// appear within about 5 or 6
						// seconds, we reset it.
						static int count = 0;
						Sleep(100);
						if (++count == 50) {
							count = 0;
							resetPipe(i, true);
						}
					} else
						LYXERR0("LyXComm: " << errormsg());
					continue;
				}
				pipe_[i].state = i < MAX_PIPES ? READING_STATE
							       : WRITING_STATE;
				break;

			case READING_STATE:
				// Pending read operation
				LASSERT(i < MAX_PIPES, /**/);
				if (!success || status == 0) {
					resetPipe(i);
					continue;
				}
				pipe_[i].nbytes = status;
				pipe_[i].state = WRITING_STATE;
				break;

			case WRITING_STATE:
				// Pending write operation
				LASSERT(i == MAX_PIPES, /**/);
				if (!success || status != writebuf_.length()) {
					resetPipe(i);
					continue;
				}
				break;
			}
		}

		// Operate according to the pipe state
		switch (pipe_[i].state) {
		case READING_STATE:
			// The pipe instance is connected to a client
			// and is ready to read a request.
			LASSERT(i < MAX_PIPES, /**/);
			success = ReadFile(pipe_[i].handle,
					pipe_[i].pipebuf, PIPE_BUFSIZE - 1,
					&pipe_[i].nbytes, &pipe_[i].overlap);

			if (success && pipe_[i].nbytes != 0) {
				// The read operation completed successfully.
				pipe_[i].pending_io = false;
				pipe_[i].state = WRITING_STATE;
				continue;
			}

			if (!success && GetLastError() == ERROR_IO_PENDING) {
				// The read operation is still pending.
				pipe_[i].pending_io = true;
				continue;
			}

			// Client closed connection (ERROR_BROKEN_PIPE) or
			// an error occurred; in either case, reset the pipe.
			if (GetLastError() != ERROR_BROKEN_PIPE) {
				LYXERR0("LyXComm: " << errormsg());
				if (!readbuf_[i].empty()) {
					LYXERR0("LyXComm: truncated command: "
						<< readbuf_[i]);
					readbuf_[i].erase();
				}
				resetPipe(i, true);
			} else
				resetPipe(i);
			break;

		case WRITING_STATE:
			if (i < MAX_PIPES) {
				// The request was successfully read
				// from the client; commit it.
				ReadReadyEvent * event = new ReadReadyEvent(i);
				QCoreApplication::postEvent(this,
						static_cast<QEvent *>(event));
				// Wait for completion
				while (pipe_[i].nbytes && !checkStopServer())
					Sleep(100);
				pipe_[i].pending_io = false;
				pipe_[i].state = READING_STATE;
				continue;
			}
			// Let's see whether we have a reply.
			if (writebuf_.length() == 0) {
				// No, nothing to do.
				pipe_[i].pending_io = false;
				continue;
			}
			// Yep, deliver it.
			success = WriteFile(pipe_[i].handle,
					writebuf_.c_str(), writebuf_.length(),
					&status, &pipe_[i].overlap);

			if (success && status == writebuf_.length()) {
				// The write operation completed successfully.
				writebuf_.erase();
				pipe_[i].pending_io = false;
				resetPipe(i);
				continue;
			}

			if (!success && (GetLastError() == ERROR_IO_PENDING)) {
				// The write operation is still pending.
				pipe_[i].pending_io = true;
				continue;
			}

			// Client closed connection (ERROR_NO_DATA) or
			// an error occurred; in either case, reset the pipe.
			if (GetLastError() != ERROR_NO_DATA) {
				LYXERR0("LyXComm: Error sending message: "
					<< writebuf_ << '\n' << errormsg());
				resetPipe(i, true);
			} else
				resetPipe(i);
			break;
		}
	}

	ready_ = false;
	closeHandles(MAX_PIPES + 1);
}


void LyXComm::closeHandles(DWORD index)
{
	for (int i = 0; i <= index; ++i) {
		if (event_[i]) {
			ResetEvent(event_[i]);
			CloseHandle(event_[i]);
		}
		if (pipe_[i].handle != INVALID_HANDLE_VALUE)
			CloseHandle(pipe_[i].handle);
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


BOOL LyXComm::checkStopServer()
{
	return WaitForSingleObject(stopserver_, 0) == WAIT_OBJECT_0;
}


void LyXComm::startPipe(DWORD index)
{
	pipe_[index].pending_io = false;

	// Overlapped ConnectNamedPipe should return zero.
	if (ConnectNamedPipe(pipe_[index].handle, &pipe_[index].overlap)) {
		// FIXME: What to do? Maybe the pipe server should be reset.
		LYXERR0("LyXComm: Could not connect pipe "
			<< external_path(pipeName(index)) << '\n'
			<< errormsg());
		return;
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
	default:
		// Anything else is an error.
		// FIXME: What to do? Maybe the pipe server should be reset.
		LYXERR0("LyXComm: An error occurred while connecting pipe "
			<< external_path(pipeName(index)) << '\n'
			<< errormsg());
	}
}


void LyXComm::resetPipe(DWORD index, bool close_handle)
{
	// This method is called when an error occurs or when a client
	// closes the connection. We first disconnect the pipe instance,
	// then reconnect it, ready to wait for another client.

	if (!DisconnectNamedPipe(pipe_[index].handle)) {
		LYXERR0("LyXComm: Could not disconnect pipe "
			<< external_path(pipeName(index)) << '\n'
			<< errormsg());
		// What to do now? Let's try whether re-creating the pipe helps.
		close_handle = true;
	}
	if (close_handle) {
		DWORD const open_mode = index < MAX_PIPES ?
				PIPE_ACCESS_INBOUND : PIPE_ACCESS_OUTBOUND;
		string const name = external_path(pipeName(index));

		CloseHandle(pipe_[index].handle);

		pipe_[index].handle = CreateNamedPipe(name.c_str(),
				open_mode | FILE_FLAG_OVERLAPPED, PIPE_WAIT,
				MAX_PIPES, PIPE_BUFSIZE, PIPE_BUFSIZE,
				PIPE_TIMEOUT, NULL);

		if (pipe_[index].handle == INVALID_HANDLE_VALUE) {
			LYXERR0("LyXComm: Could not reset pipe "
				<< name << '\n' << errormsg());
			return;
		}
		if (index == MAX_PIPES)
			writebuf_.erase();
		else
			readbuf_[index].erase();
	}
	startPipe(index);
	pipe_[index].state = pipe_[index].pending_io ?
			CONNECTING_STATE : (index < MAX_PIPES ? READING_STATE
							      : WRITING_STATE);
}


void LyXComm::openConnection()
{
	LYXERR(Debug::LYXSERVER, "LyXComm: Opening connection");

	// If we are up, that's an error
	if (ready_) {
		LYXERR0("LyXComm: Already connected");
		return;
	}
	// We assume that we don't make it
	ready_ = false;

	if (pipename_.empty()) {
		LYXERR(Debug::LYXSERVER, "LyXComm: server is disabled, nothing to do");
		return;
	}

	// Check whether the pipe name is being used by some other program.
	if (!stopserver_ && WaitNamedPipe(inPipeName().c_str(), 0)) {
		LYXERR0("LyXComm: Pipe " << external_path(inPipeName())
			<< " already exists.\nMaybe another instance of LyX"
			   " is using it.");
		pipename_.erase();
		return;
	}

	// Manual-reset event, initial state = not signaled
	stopserver_ = CreateEvent(NULL, TRUE, FALSE, NULL);
	server_thread_ = CreateThread(NULL, 0, pipeServerWrapper,
				     static_cast<void *>(this), 0, NULL);
	if (!server_thread_) {
		LYXERR0("LyXComm: Could not create pipe server thread\n"
			<< errormsg());
		pipename_.erase();
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

	emergencyCleanup();
}


void LyXComm::emergencyCleanup()
{
	if (ready_) {
		SetEvent(stopserver_);
		// Wait for the pipe server to finish
		WaitForSingleObject(server_thread_, INFINITE);
		CloseHandle(server_thread_);
		ResetEvent(stopserver_);
		CloseHandle(stopserver_);
	}
}


void LyXComm::read_ready(DWORD inpipe)
{
	// Turn the pipe buffer into a C string
	DWORD const nbytes = pipe_[inpipe].nbytes;
	pipe_[inpipe].pipebuf[nbytes] = '\0';

	readbuf_[inpipe] += rtrim(pipe_[inpipe].pipebuf, "\r");

	// Commit any commands read
	while (readbuf_[inpipe].find('\n') != string::npos) {
		// split() grabs the entire string if
		// the delim /wasn't/ found. ?:-P
		string cmd;
		readbuf_[inpipe] = split(readbuf_[inpipe], cmd, '\n');
		cmd = rtrim(cmd, "\r");
		LYXERR(Debug::LYXSERVER, "LyXComm: status:" << nbytes
			<< ", readbuf_:" << readbuf_[inpipe]
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
		LYXERR0("LyXComm: Request to send empty string. Ignoring.");
		return;
	}

	LYXERR(Debug::LYXSERVER, "LyXComm: Sending '" << msg << '\'');

	if (pipename_.empty())
		return;

	if (!ready_) {
		LYXERR0("LyXComm: Pipes are closed. Could not send " << msg);
		return;
	}

	// Wait a couple of secs for completion of a previous write operation.
	for (int count = 0; writebuf_.length() && count < 20; ++count)
		Sleep(100);

	if (!writebuf_.length()) {
		writebuf_ = msg;
		// Tell the pipe server he has a job to do.
		SetEvent(pipe_[MAX_PIPES].overlap.hEvent);
	} else {
		// Nope, output pipe is still busy. Most likely, a bad client
		// did not care to read the answer (JabRef is one such client).
		// Let's do a reset, otherwise the output pipe could remain
		// stalled if the pipe server failed to reset it.
		// This will remedy the output pipe stall, but the client will
		// get a broken pipe error.
		LYXERR0("LyXComm: Error sending message: " << msg
			<< "\nLyXComm: Output pipe is stalled\n"
		           "LyXComm: Resetting connection");
		closeConnection();
		if (!checkStopServer())
			openConnection();
	}
}


string const LyXComm::pipeName(DWORD index) const
{
	return index < MAX_PIPES ? inPipeName() : outPipeName();
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
			boost::bind(&LyXComm::read_ready, this));
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

	if (FileName(filename).removeFile() < 0) {
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

	if (pipename_.empty()) return;

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

Server::Server(LyXFunc * f, string const & pipes)
	: numclients_(0), func_(f), pipes_(pipes, this, &ServerCallback)
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
	using namespace std;
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
			// connect to the lyxfunc in the single LyXView we
			// support currently. (Lgb)

			func_->dispatch(FuncRequest(lyxaction.lookupFunc(cmd), arg));
			string const rval = to_utf8(func_->getMessage());

			// all commands produce an INFO or ERROR message
			// in the output pipe, even if they do not return
			// anything. See chapter 4 of Customization doc.
			string buf;
			if (func_->errorStat())
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
