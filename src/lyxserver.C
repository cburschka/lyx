// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

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

#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxserver.h"
#include "lyx_main.h"
#include "debug.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"

#ifdef __EMX__
#include <cstdlib>
#include <io.h>
#define OS2EMX_PLAIN_CHAR
#define INCL_DOSNMPIPES
#define INCL_DOSERRORS
#include <os2.h>
#include "os2_errortable.h"
#endif

using std::endl;

// provide an empty mkfifo() if we do not have one. This disables the
// lyxserver. 
#ifndef HAVE_MKFIFO
int mkfifo(char const * __path, mode_t __mode) {
        return 0;
}
#endif


/* === variables ========================================================= */

extern LyXAction lyxaction;

// C wrapper
extern "C" void C_LyXComm_callback(int fd, void *v);


// LyXComm class
 
 // Open pipes
void LyXComm::openConnection() {
	lyxerr[Debug::LYXSERVER] << "LyXComm: Opening connection" << endl;
       
	// If we are up, that's an error
	if (ready) {
		lyxerr << "LyXComm: Already connected" << endl;
		return;
	}
	// We assume that we don't make it
	ready = false;
 
	if (pipename.empty()) return;

	// --- prepare input pipe ---------------------------------------
 
	string tmp = pipename + ".in";
       
#ifdef __EMX__
	HPIPE fd;
	APIRET rc;
	int errnum;
	// Try create one instance of named pipe with the mode O_RDONLY|O_NONBLOCK.
	// The current emx implementation of access() won't work with pipes.
	rc = DosCreateNPipe(tmp.c_str(), &fd, NP_ACCESS_INBOUND,
		NP_NOWAIT|0x01, 0600, 0600, 0);
	if (rc == ERROR_PIPE_BUSY) {
#else
	if (::access(tmp.c_str(), F_OK) == 0) {
#endif
		lyxerr << "LyXComm: Pipe " << tmp << " already exists.\n"
		       << "If no other LyX program is active, please delete"
			" the pipe by hand and try again." << endl;
		pipename.erase();
		return;
	}
#ifndef __EMX__
	if (::mkfifo(tmp.c_str(), 0600) < 0) {
		lyxerr << "LyXComm: Could not create pipe " << tmp << '\n'
		       << strerror(errno) << endl;
		return;
	};
	infd = ::open(tmp.c_str(), O_RDONLY|O_NONBLOCK);
#else
	if (rc != NO_ERROR) {
		errnum = TranslateOS2Error(rc);
		lyxerr <<"LyXComm: Could not create pipe " << tmp
		       << strerror(errnum) << endl;
		return;
	};
	// Listen to it.
	rc = DosConnectNPipe(fd);
	if (rc != NO_ERROR && rc != ERROR_PIPE_NOT_CONNECTED) {
		errnum = TranslateOS2Error(rc);
		lyxerr <<"LyXComm: Could not create pipe " << tmp
		       << strerror(errnum) << endl;
		return;
	};
	// Imported handles can be used both with OS/2 APIs and emx
	// library functions. 
	infd = _imphandle(fd);
#endif
	if (infd < 0) {
		lyxerr << "LyXComm: Could not open pipe " << tmp << '\n'
		       << strerror(errno) << endl;
		return;
	}
	fl_add_io_callback(infd, FL_READ, C_LyXComm_callback, this);
 
	// --- prepare output pipe ---------------------------------------
 
	tmp = pipename + ".out";
       
#ifndef __EMX__       
	if (::access(tmp.c_str(), F_OK) == 0) {
#else
	rc = DosCreateNPipe(tmp.c_str(), &fd, NP_ACCESS_DUPLEX,
		NP_NOWAIT|0x01, 0600, 0600, 0);

	if (rc == ERROR_PIPE_BUSY) {
#endif
		lyxerr << "LyXComm: Pipe " << tmp << " already exists.\n"
		       << "If no other LyX program is active, please delete"
			" the pipe by hand and try again." << endl;
		pipename.erase();
		return;
	}
#ifndef __EMX__
	if (::mkfifo(tmp.c_str(), 0600) < 0) {
		lyxerr << "LyXComm: Could not create pipe " << tmp << '\n'
		       << strerror(errno) << endl;
		return;
	};
	if (::access(tmp.c_str(), F_OK) != 0) {
		lyxerr << "LyXComm: Pipe " << tmp
		       << " does not exist" << endl;
		return;
	}
	outfd = ::open(tmp.c_str(), O_RDWR);
#else
	if (rc != NO_ERROR) {
		errnum = TranslateOS2Error(rc);
		lyxerr << "LyXComm: Could not create pipe " << tmp << '\n'
		       << strerror(errnum) << endl;
		return;
	}
	rc = DosConnectNPipe(fd);
	if (rc == ERROR_BAD_PIPE) {
		lyxerr << "LyXComm: Pipe " << tmp
		       << " does not exist" << endl;
		return;
	}
	if (rc != NO_ERROR && rc != ERROR_PIPE_NOT_CONNECTED) {
		errnum = TranslateOS2Error(rc);
		lyxerr << "LyXComm: Could not create pipe " << tmp << '\n'
		       << strerror(errnum) << endl;
		return;
	}
	outfd = _imphandle(fd);
#endif
	if (outfd < 0) {
		lyxerr << "LyXComm: Could not open pipe " << tmp << '\n'
		       << strerror(errno) << endl;
		return;
	}
	if (fcntl(outfd, F_SETFL, O_NONBLOCK) < 0) {
		lyxerr << "LyXComm: Could not set flags on pipe " << tmp
		       << '\n' << strerror(errno) << endl;
		return;
	};
	// We made it!
	ready = true;
	lyxerr[Debug::LYXSERVER] << "LyXComm: Connection established" << endl;
}
 
/// Close pipes
void LyXComm::closeConnection() {
#ifdef __EMX__
	APIRET rc;
	int errnum;
#endif
       	lyxerr[Debug::LYXSERVER] << "LyXComm: Closing connection" << endl;

	if (pipename.empty()) {
	        return;
	}

	if (!ready) {
		lyxerr << "LyXComm: Already disconnected" << endl;
		return;
	}
 
	if (infd > -1) {
		fl_remove_io_callback(infd, FL_READ, C_LyXComm_callback);
 
		string tmp = pipename + ".in";
#ifdef __EMX__		// Notify the operating system.
		rc = DosDisConnectNPipe(infd);
		if (rc != NO_ERROR) {
			errnum = TranslateOS2Error(rc);
			lyxerr << "LyXComm: Could not disconnect pipe " << tmp
			       << '\n' << strerror(errnum) << endl;
			return;
		}
#endif
		if (close(infd) < 0) {
			lyxerr << "LyXComm: Could not close pipe " << tmp
			       << '\n' << strerror(errno) << endl;
		}
#ifndef __EMX__		// OS/2 named pipes will be automatically removed.
		if (lyx::unlink(tmp) < 0){
			lyxerr << "LyXComm: Could not remove pipe " << tmp
			       << '\n' << strerror(errno) << endl;
		};
#endif
	}
	if (outfd > -1) {
		string tmp = pipename + ".out";
#ifdef __EMX__
		rc = DosDisConnectNPipe(outfd);
		if (rc != NO_ERROR) {
			errnum = TranslateOS2Error(rc);
			lyxerr << "LyXComm: Could not disconnect pipe " << tmp
			       << '\n' << strerror(errnum) << endl;
			return;
		}
#endif
		if (::close(outfd) < 0) {
			lyxerr << "LyXComm: Could not close pipe " << tmp
			       << '\n' << strerror(errno) << endl;
		}
#ifndef __EMX__
		if (lyx::unlink(tmp) < 0){
			lyxerr << "LyXComm: Could not remove pipe " << tmp
			       << '\n' << strerror(errno) << endl;
		};
#endif
	}
	ready = false;
}
 
// Receives messages and sends then to client
void LyXComm::callback(int fd, void *v)
{
	LyXComm * c = static_cast<LyXComm*>(v);
 
	if (lyxerr.debugging(Debug::LYXSERVER)) {
		lyxerr << "LyXComm: Receiving from fd " << fd << endl;
	}
 
        const int CMDBUFLEN = 100;
	char charbuf[CMDBUFLEN];
  	string cmd;
// nb! make lsbuf a class-member for multiple sessions
	static string lsbuf;

	errno = 0;
	int status;
	// the single = is intended here.
	while((status = read(fd, charbuf, CMDBUFLEN-1)))
	{// break and return in loop
		if (status > 0) // got something
		{
			charbuf[status]= '\0'; // turn it into a c string
			lsbuf += strip(charbuf, '\r');
			// commit any commands read
			while(lsbuf.find('\n') != string::npos) // while still
							// commands
							// left 
			{
				// split() grabs the entire string if
				// the delim /wasn't/ found. ?:-P 
				lsbuf= split(lsbuf, cmd,'\n');
				lyxerr[Debug::LYXSERVER]
					<< "LyXComm: status:" << status
					<< ", lsbuf:" << lsbuf 
					<< ", cmd:" << cmd << endl;
				if (!cmd.empty())
					c->clientcb(c->client, cmd); 
				        //\n or not \n?
			}
		}
		if (errno == EAGAIN)
		{  // EAGAIN is not really an error , it means we're
		   // only reading too fast for the writing process on
		   // the other end of the pipe. 
			errno = 0;
			return; // up to libforms select-loop (*crunch*)
		}
		if (errno != 0 )
		{
			lyxerr << "LyXComm: " << strerror(errno) << endl;
			if (!lsbuf.empty())
			{
				lyxerr << "LyxComm: truncated command: " 
				       << lsbuf << endl;
				lsbuf.erase();
			}
			break; // reset connection
		}
	}
	c->closeConnection();
	c->openConnection();
	errno= 0;
}

extern "C" void C_LyXComm_callback(int fd, void *v)
{
	LyXComm::callback(fd, v);
}

 
void LyXComm::send(string const & msg) {
	if (msg.empty()) {
		lyxerr << "LyXComm: Request to send empty string. Ignoring."
		       << endl;
		return;
	}

	if (lyxerr.debugging(Debug::LYXSERVER)) {
		lyxerr << "LyXComm: Sending '" << msg << '\'' << endl;
	}

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
#ifdef __EMX__
	APIRET rc;
	int errnum;
	rc = DosResetBuffer(outfd);	// To avoid synchronization problems.
	if (rc != NO_ERROR) {
		errnum = TranslateOS2Error(rc);
		lyxerr << "LyXComm: Message could not be flushed: " << msg
		       << '\n' << strerror(errnum) << endl;
	}
#endif
}


// LyXServer class

LyXServer::~LyXServer()
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

void LyXServer::callback(LyXServer * serv, string const & msg)
{
	lyxerr[Debug::LYXSERVER] << "LyXServer: Received: '"
				 << msg << '\'' << endl;
 
	char const * p = msg.c_str();
 
	// --- parse the string --------------------------------------------
	//
	//  Format: LYXCMD:<client>:<func>:<argstring>\n
	//
	bool server_only = false;
	while(*p) {
		// --- 1. check 'header' ---

	        if (compare(p, "LYXSRV:", 7) == 0) {
			server_only = true; 
		} else if (0 != compare(p, "LYXCMD:", 7)) {
			lyxerr << "LyXServer: Unknown request" << endl;
			return;
		}
		p += 7;
		
		// --- 2. for the moment ignore the client name ---
		string client;
		while(*p && *p != ':')
			client += char(*p++);
		if (*p == ':') ++p;
		if (!*p) return;
		
		// --- 3. get function name ---
		string cmd;
		while(*p && *p != ':')
			cmd += char(*p++);
		
		// --- 4. parse the argument ---
		string arg;
		if (!server_only && *p == ':' && *(++p)) {
			while(*p && *p != '\n')
				arg += char(*p++);
			if (*p) ++p;
		}
 
		lyxerr[Debug::LYXSERVER]
			<< "LyXServer: Client: '" << client
			<< "' Command: '" << cmd
			<< "' Argument: '" << arg << '\'' << endl;
		
		// --- lookup and exec the command ------------------
 
		if (server_only) {
			string buf;
			// return the greeting to inform the client that 
			// we are listening.
			if (cmd == "hello") {
				// One more client
				if (serv->numclients == MAX_CLIENTS){ //paranoid check
					lyxerr[Debug::LYXSERVER]
						<< "LyXServer: too many clients..."
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
				lyxerr[Debug::LYXSERVER]
					<< "LyXServer: Greeting "
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
					lyxerr[Debug::LYXSERVER]
						<< "LyXServer: Client "
						<< client << " said goodbye"
						<< endl;
				} else {
					lyxerr[Debug::LYXSERVER]
						<< "LyXServer: ignoring bye messge from unregistered client"
						<< client << endl;
				}
			} else {
				lyxerr <<"LyXServer: Undefined server command "
				       << cmd << "." << endl;
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

			int action = lyxaction.LookupFunc(cmd);
			//int action = -1;
			string rval, buf;
		    
			if (action>= 0) {
				rval = serv->func->Dispatch(action, arg);
			} else {
				rval = "Unknown command";
			}

      //modified june 1999 stefano@zool.su.se:
			//all commands produce an INFO or ERROR message
			//in the output pipe, even if they do not return
			//anything. See chapter 4 of Customization doc.
			if (action<0 || serv->func->errorStat())
				buf = "ERROR:";
			else
				buf = "INFO:";
			buf += string(client) + ":" + cmd 	+ ":" + rval + "\n";
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
   Function  : LyxNotifyClient
   Called by : WorkAreaKeyPress
   Purpose   : send a notify messge to a client
   Parameters: s - string to send
   Returns   : nothing
   \* ---F------------------------------------------------------------------- */

void LyXServer::notifyClient(string const & s)
{
	string buf = string("NOTIFY:") + s + "\n";
	pipes.send(buf);
}

