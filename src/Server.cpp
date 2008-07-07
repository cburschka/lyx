/**
 * \file Server.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
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
#include "support/lstrings.h"

#include <boost/bind.hpp>

#include <cerrno>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <fcntl.h>

using namespace std;
using namespace lyx::support;

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// LyXComm
//
/////////////////////////////////////////////////////////////////////

#if !defined (HAVE_MKFIFO)
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
	FileName const filename(file);
	if (::access(filename.toFilesystemEncoding().c_str(), F_OK) == 0) {
		lyxerr << "LyXComm: Pipe " << filename << " already exists.\n"
		       << "If no other LyX program is active, please delete"
			" the pipe by hand and try again." << endl;
		pipename_.erase();
		return -1;
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
		}
		if (errno == EAGAIN) {
			errno = 0;
			return;
		}
		if (errno != 0) {
			LYXERR0("LyXComm: " << strerror(errno));
			if (!read_buffer_.empty()) {
				LYXERR0("LyXComm: truncated command: " << read_buffer_);
				read_buffer_.erase();
			}
			break; // reset connection
		}
	}

	// The connection gets reset in errno != EAGAIN
	// Why does it need to be reset if errno == 0?
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
