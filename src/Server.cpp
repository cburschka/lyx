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
#include "debug.h"
#include "FuncRequest.h"
#include "LyXAction.h"
#include "LyXFunc.h"
#include "frontends/Application.h"

#include "support/FileName.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>

#include <cerrno>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <fcntl.h>


namespace lyx {

using support::compare;
using support::FileName;
using support::rtrim;
using support::split;
using support::unlink;

using std::endl;
using std::string;


#if !defined (HAVE_MKFIFO)
// We provide a stub class that disables the lyxserver.

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


void LyXComm::openConnection()
{
	LYXERR(Debug::LYXSERVER) << "LyXComm: Opening connection" << endl;

	// If we are up, that's an error
	if (ready) {
		lyxerr << "LyXComm: Already connected" << endl;
		return;
	}
	// We assume that we don't make it
	ready = false;

	if (pipename.empty()) {
		LYXERR(Debug::LYXSERVER)
			<< "LyXComm: server is disabled, nothing to do"
			<< endl;
		return;
	}

	if ((infd = startPipe(inPipeName(), false)) == -1)
		return;

	if ((outfd = startPipe(outPipeName(), true)) == -1) {
		endPipe(infd, inPipeName(), false);
		return;
	}

	if (fcntl(outfd, F_SETFL, O_NONBLOCK) < 0) {
		lyxerr << "LyXComm: Could not set flags on pipe " << outPipeName()
		       << '\n' << strerror(errno) << endl;
		return;
	}

	// We made it!
	ready = true;
	LYXERR(Debug::LYXSERVER) << "LyXComm: Connection established" << endl;
}


/// Close pipes
void LyXComm::closeConnection()
{
	LYXERR(Debug::LYXSERVER) << "LyXComm: Closing connection" << endl;

	if (pipename.empty()) {
		LYXERR(Debug::LYXSERVER)
			<< "LyXComm: server is disabled, nothing to do"
			<< endl;
		return;
	}

	if (!ready) {
		lyxerr << "LyXComm: Already disconnected" << endl;
		return;
	}

	endPipe(infd, inPipeName(), false);
	endPipe(outfd, outPipeName(), true);

	ready = false;
}


int LyXComm::startPipe(string const & file, bool write)
{
	FileName const filename(file);
	if (::access(filename.toFilesystemEncoding().c_str(), F_OK) == 0) {
		lyxerr << "LyXComm: Pipe " << filename << " already exists.\n"
		       << "If no other LyX program is active, please delete"
			" the pipe by hand and try again." << endl;
		pipename.erase();
		return -1;
	}

	if (::mkfifo(filename.toFilesystemEncoding().c_str(), 0600) < 0) {
		lyxerr << "LyXComm: Could not create pipe " << filename << '\n'
		       << strerror(errno) << endl;
		return -1;
	};
	int const fd = ::open(filename.toFilesystemEncoding().c_str(),
			      write ? (O_RDWR) : (O_RDONLY|O_NONBLOCK));

	if (fd < 0) {
		lyxerr << "LyXComm: Could not open pipe " << filename << '\n'
		       << strerror(errno) << endl;
		unlink(filename);
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

	if (!write) {
		theApp()->unregisterSocketCallback(fd);
	}

	if (::close(fd) < 0) {
		lyxerr << "LyXComm: Could not close pipe " << filename
		       << '\n' << strerror(errno) << endl;
	}

	if (unlink(FileName(filename)) < 0) {
		lyxerr << "LyXComm: Could not remove pipe " << filename
		       << '\n' << strerror(errno) << endl;
	};

	fd = -1;
}


void LyXComm::emergencyCleanup()
{
	if (!pipename.empty()) {
		endPipe(infd, inPipeName(), false);
		endPipe(outfd, outPipeName(), true);
	}
}


// Receives messages and sends then to client
void LyXComm::read_ready()
{
	// nb! make read_buffer_ a class-member for multiple sessions
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
	while ((status = ::read(infd, charbuf, charbuf_size - 1))) {

		if (status > 0) {
			charbuf[status] = '\0'; // turn it into a c string
			read_buffer_ += rtrim(charbuf, "\r");
			// commit any commands read
			while (read_buffer_.find('\n') != string::npos) {
				// split() grabs the entire string if
				// the delim /wasn't/ found. ?:-P
				string cmd;
				read_buffer_= split(read_buffer_, cmd,'\n');
				LYXERR(Debug::LYXSERVER)
					<< "LyXComm: status:" << status
					<< ", read_buffer_:" << read_buffer_
					<< ", cmd:" << cmd << endl;
				if (!cmd.empty())
					clientcb(client, cmd);
					//\n or not \n?
			}
		} else {
			if (errno == EAGAIN) {
				// Nothing to read, continue
				errno = 0;
				return;
			}
			// An error occurred, better bailing out
			lyxerr << "LyXComm: " << strerror(errno) << endl;
			if (!read_buffer_.empty()) {
				lyxerr << "LyXComm: truncated command: "
				       << read_buffer_ << endl;
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
		lyxerr << "LyXComm: Request to send empty string. Ignoring."
		       << endl;
		return;
	}

	LYXERR(Debug::LYXSERVER) << "LyXComm: Sending '" << msg << '\'' << endl;

	if (pipename.empty()) return;

	if (!ready) {
		lyxerr << "LyXComm: Pipes are closed. Could not send "
		       << msg << endl;
	} else if (::write(outfd, msg.c_str(), msg.length()) < 0) {
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
	return pipename + string(".in");
}


string const LyXComm::outPipeName() const
{
	return pipename + string(".out");
}


// Server class

Server::~Server()
{
	// say goodbye to clients so they stop sending messages
	// modified june 1999 by stefano@zool.su.se to send as many bye
	// messages as there are clients, each with client's name.
	string message;
	for (int i= 0; i<numclients; ++i) {
		message = "LYXSRV:" + clients[i] + ":bye\n";
		pipes.send(message);
	}
}


/* ---F+------------------------------------------------------------------ *\
   Function  : ServerCallback
    Called by : LyXComm
    Purpose   : handle data gotten from communication
\* ---F------------------------------------------------------------------- */

void Server::callback(Server * serv, string const & msg)
{
	LYXERR(Debug::LYXSERVER) << "Server: Received: '"
				 << msg << '\'' << endl;

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
		if (*p == ':') ++p;
		if (!*p) return;

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

		LYXERR(Debug::LYXSERVER)
			<< "Server: Client: '" << client
			<< "' Command: '" << cmd
			<< "' Argument: '" << arg << '\'' << endl;

		// --- lookup and exec the command ------------------

		if (server_only) {
			string buf;
			// return the greeting to inform the client that
			// we are listening.
			if (cmd == "hello") {
				// One more client
				if (serv->numclients == MAX_CLIENTS) { //paranoid check
					LYXERR(Debug::LYXSERVER)
						<< "Server: too many clients..."
						<< endl;
					return;
				}
				int i= 0; //find place in clients[]
				while (!serv->clients[i].empty()
				       && i<serv->numclients)
					++i;
				serv->clients[i] = client;
				serv->numclients++;
				buf = "LYXSRV:" + client + ":hello\n";
				LYXERR(Debug::LYXSERVER)
					<< "Server: Greeting "
					<< client << endl;
				serv->pipes.send(buf);
			} else if (cmd == "bye") {
				// If clients == 0 maybe we should reset the pipes
				// to prevent fake callbacks
				int i = 0; //look if client is registered
				for (; i < serv->numclients; ++i) {
					if (serv->clients[i] == client) break;
				}
				if (i < serv->numclients) {
					serv->numclients--;
					serv->clients[i].erase();
					LYXERR(Debug::LYXSERVER)
						<< "Server: Client "
						<< client << " said goodbye"
						<< endl;
				} else {
					LYXERR(Debug::LYXSERVER)
						<< "Server: ignoring bye messge from unregistered client"
						<< client << endl;
				}
			} else {
				lyxerr <<"Server: Undefined server command "
				       << cmd << '.' << endl;
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


			serv->func->dispatch(FuncRequest(lyxaction.lookupFunc(cmd), arg));
			string const rval = to_utf8(serv->func->getMessage());

			//modified june 1999 stefano@zool.su.se:
			//all commands produce an INFO or ERROR message
			//in the output pipe, even if they do not return
			//anything. See chapter 4 of Customization doc.
			string buf;
			if (serv->func->errorStat())
				buf = "ERROR:";
			else
				buf = "INFO:";
			buf += client + ':' + cmd + ':' +  rval + '\n';
			serv->pipes.send(buf);

			// !!! we don't do any error checking -
			//  if the client won't listen, the
			//  message is lost and others too
			//  maybe; so the client should empty
			//  the outpipe before issuing a request.

			// not found
		}
	}  /* while *p */
}


/* ---F+------------------------------------------------------------------ *\
   Function  : LyXNotifyClient
   Called by : WorkAreaKeyPress
   Purpose   : send a notify messge to a client
   Parameters: s - string to send
   Returns   : nothing
   \* ---F------------------------------------------------------------------- */

void Server::notifyClient(string const & s)
{
	string buf = string("NOTIFY:") + s + "\n";
	pipes.send(buf);
}


} // namespace lyx
