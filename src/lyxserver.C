// -*- C++ -*-
/* This file is part of
 * ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

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

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxserver.h"
#include "lyxfunc.h"
#include "lyx_main.h"
#include "error.h"

#ifdef __EMX__
#include <stdlib.h>
#include <io.h>
#define OS2EMX_PLAIN_CHAR
#define INCL_DOSNMPIPES
#define INCL_DOSERRORS
#include <os2.h>
#include "os2_errortable.h"
#endif

// provide an empty mkfifo() if we do not have one. This disables the
// lyxserver. 
#ifndef HAVE_MKFIFO
int	mkfifo( char *__path, mode_t __mode ) {
	return 0;
}
#endif


// 	$Id: lyxserver.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: lyxserver.C,v 1.1 1999/09/27 18:44:38 larsbj Exp $";
#endif /* lint */
	
/* === variables ========================================================= */

extern LyXAction lyxaction;

// LyXComm class
 
 // Open pipes
void LyXComm::openConnection() {
	lyxerr.debug("LyXComm: Opening connection", Error::LYXSERVER);
       
	// If we are up, that's an error
	if (ready) {
		lyxerr.print("LyXComm: Already connected");
		return;
	}
	// We assume that we don't make it
	ready = false;
 
	if (pipename.empty()) return;

	// --- prepare input pipe ---------------------------------------
 
	LString tmp = pipename + ".in";
       
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
	if (access(tmp.c_str(), F_OK) == 0) {
#endif
		lyxerr.print("LyXComm: Pipe " + tmp + " already exists.");
		lyxerr.print("If no other LyX program is active, please delete"
			     " the pipe by hand and try again.");
		pipename = LString();
		return;
	}
#ifndef __EMX__
	if (mkfifo(tmp.c_str(), 0600) < 0) {
		lyxerr.print("LyXComm: Could not create pipe " + tmp);
		lyxerr.print(strerror(errno));
		return;
	};
	infd = open(tmp.c_str(), O_RDONLY|O_NONBLOCK);
#else
	if (rc != NO_ERROR) {
		errnum = TranslateOS2Error(rc);
		lyxerr.print("LyXComm: Could not create pipe " + tmp);
		lyxerr.print(strerror(errnum));
		return;
	};
	// Listen to it.
	rc = DosConnectNPipe(fd);
	if (rc != NO_ERROR && rc != ERROR_PIPE_NOT_CONNECTED) {
		errnum = TranslateOS2Error(rc);
		lyxerr.print("LyXComm: Could not create pipe " + tmp);
		lyxerr.print(strerror(errnum));
		return;
	};
	// Imported handles can be used both with OS/2 APIs and emx
	// library functions. 
	infd = _imphandle(fd);
#endif
	if (infd < 0) {
		lyxerr.print("LyXComm: Could not open pipe " + tmp);
		lyxerr.print(strerror(errno));
		return;
	}
	fl_add_io_callback(infd, FL_READ, callback, (void*)this);
 
	// --- prepare output pipe ---------------------------------------
 
	tmp = pipename + ".out";
       
#ifndef __EMX__       
	if (access(tmp.c_str(), F_OK) == 0) {
#else
	rc = DosCreateNPipe(tmp.c_str(), &fd, NP_ACCESS_DUPLEX,
		NP_NOWAIT|0x01, 0600, 0600, 0);

	if (rc == ERROR_PIPE_BUSY) {
#endif
		lyxerr.print("LyXComm: Pipe " + tmp + " already exists.");
		lyxerr.print("If no other LyX program is active, please delete"
			     " the pipe by hand and try again.");
		pipename = LString();
		return;
	}
#ifndef __EMX__
	if (mkfifo(tmp.c_str(), 0600) < 0) {
		lyxerr.print("LyXComm: Could not create pipe " + tmp);
		lyxerr.print(strerror(errno));
		return;
	};
	if (access(tmp.c_str(), F_OK) != 0) {
		lyxerr.print("LyXComm: Pipe " + tmp + " does not exist");
		return;
	}
	outfd = open(tmp.c_str(), O_RDWR);
#else
	if (rc != NO_ERROR) {
		errnum = TranslateOS2Error(rc);
		lyxerr.print("LyXComm: Could not create pipe " + tmp);
		lyxerr.print(strerror(errnum));
		return;
	}
	rc = DosConnectNPipe(fd);
	if (rc == ERROR_BAD_PIPE) {
		lyxerr.print("LyXComm: Pipe " + tmp + " does not exist");
		return;
	}
	if (rc != NO_ERROR && rc != ERROR_PIPE_NOT_CONNECTED) {
		errnum = TranslateOS2Error(rc);
		lyxerr.print("LyXComm: Could not create pipe " + tmp);
		lyxerr.print(strerror(errnum));
		return;
	}
	outfd = _imphandle(fd);
#endif
	if (outfd < 0) {
		lyxerr.print("LyXComm: Could not open pipe " + tmp);
		lyxerr.print(strerror(errno));
		return;
	}
	if (fcntl(outfd, F_SETFL, O_NONBLOCK) < 0) {
		lyxerr.print("LyXComm: Could not set flags on pipe " + tmp);
		lyxerr.print(strerror(errno));
		return;
	};
	// We made it!
	ready = true;
	lyxerr.debug("LyXComm: Connection established", Error::LYXSERVER);
}
 
/// Close pipes
void LyXComm::closeConnection() {
#ifdef __EMX__
	APIRET rc;
	int errnum;
#endif
       	lyxerr.debug("LyXComm: Closing connection", Error::LYXSERVER);

	if (pipename.empty()) {
	        return;
	}

	if (!ready) {
		lyxerr.print("LyXComm: Already disconnected");
		return;
	}
 
	if(infd > -1) {
		fl_remove_io_callback(infd, FL_READ, callback);
 
		LString tmp = pipename + ".in";
#ifdef __EMX__		// Notify the operating system.
		rc = DosDisConnectNPipe(infd);
		if (rc != NO_ERROR) {
			errnum = TranslateOS2Error(rc);
			lyxerr.print("LyXComm: Could not disconnect pipe " + tmp);
			lyxerr.print(strerror(errnum));
			return;
		}
#endif
		if (close(infd) < 0) {
			lyxerr.print("LyXComm: Could not close pipe " + tmp);
			lyxerr.print(strerror(errno));
		}
#ifndef __EMX__		// OS/2 named pipes will be automatically removed.
		if (unlink(tmp.c_str()) < 0){
			lyxerr.print("LyXComm: Could not remove pipe " + tmp);
			lyxerr.print(strerror(errno));
		};
#endif
	}
	if(outfd > -1) {
		LString tmp = pipename + ".out";
#ifdef __EMX__
		rc = DosDisConnectNPipe(outfd);
		if (rc != NO_ERROR) {
			errnum = TranslateOS2Error(rc);
			lyxerr.print("LyXComm: Could not disconnect pipe " + tmp);
			lyxerr.print(strerror(errnum));
			return;
		}
#endif
		if (close(outfd) < 0) {
			lyxerr.print("LyXComm: Could not close pipe " + tmp);
			lyxerr.print(strerror(errno));
		}
#ifndef __EMX__
		if (unlink(tmp.c_str()) < 0){
			lyxerr.print("LyXComm: Could not remove pipe " + tmp);
			lyxerr.print(strerror(errno));
		};
#endif
	}
	ready = false;
}
 
// Receives messages and sends then to client
void LyXComm::callback(int fd, void *v)
{
	LyXComm * c = (LyXComm *) v;
 
	if (lyxerr.debugging(Error::LYXSERVER)) {
		lyxerr.print(LString("LyXComm: Receiving from fd ") + int(fd));
	}
 
        const int CMDBUFLEN = 100;
	char charbuf[CMDBUFLEN];
  	LString cmd;
// nb! make lsbuf a class-member for multiple sessions
	static LString lsbuf;

	errno = 0;
	int status;
	// the single = is intended here.
	while((status = read(fd,charbuf,CMDBUFLEN-1)))
	{// break and return in loop
		if(status > 0) // got something
		{
			charbuf[status]='\0'; // turn it into a c string
			lsbuf += charbuf;
			lsbuf.strip('\r');
			// commit any commands read
			while(lsbuf.charPos('\n') >= 0) // while still
							// commands
							// left 
			{
				// split() grabs the entire string if
				// the delim /wasn't/ found. ?:-P 
				lsbuf.split(cmd,'\n');
				lyxerr.debug(LString("LyXComm: status:") 
					     + status + ", lsbuf:" + lsbuf 
					     + ", cmd:" + cmd, 
					     Error::LYXSERVER);
				if(!cmd.empty())
					c->clientcb(c->client, cmd); 
				        //\n or not \n?
			}
		}
		if(errno == EAGAIN)
		{  // EAGAIN is not really an error , it means we're
		   // only reading too fast for the writing process on
		   // the other end of the pipe. 
			errno = 0;
			return; // up to libforms select-loop (*crunch*)
		}
		if(errno != 0 )
		{
			lyxerr.print(LString("LyXComm: ") + strerror(errno));
			if(!lsbuf.empty())
			{
				lyxerr.print("LyxComm: truncated command: " 
					     + lsbuf);
				lsbuf.clean();
			}
			break; // reset connection
		}
	}
	c->closeConnection();
	c->openConnection();
	errno=0;
}
 
void LyXComm::send(LString const & msg) {
	if (msg.empty()) {
		lyxerr.print("LyXComm: Request to send empty string. Ignoring.");
		return;
	}

	if (lyxerr.debugging(Error::LYXSERVER)) {
		lyxerr.print("LyXComm: Sending '" + msg + '\'');
	}

	if (pipename.empty()) return;

	if (!ready) {
		lyxerr.print("LyXComm: Pipes are closed. Could not send "+ msg);
	} else if (write(outfd, msg.c_str(), msg.length()) < 0) {
		lyxerr.print("LyXComm: Error sending message: " + msg);
		lyxerr.print(strerror(errno));
		lyxerr.print("LyXComm: Resetting connection");
		closeConnection();
		openConnection();
	}
#ifdef __EMX__
	APIRET rc;
	int errnum;
	rc = DosResetBuffer(outfd);	// To avoid synchronization problems.
	if (rc != NO_ERROR) {
		errnum = TranslateOS2Error(rc);
		lyxerr.print("LyXComm: Message could not be flushed: " +msg);
		lyxerr.print(strerror(errnum));
	}
#endif
}


// LyXServer class

LyXServer::~LyXServer()
{
	// say goodbye to clients so they stop sending messages
	// modified june 1999 by stefano@zool.su.se to send as many bye
	// messages as there are clients, each with client's name.
	LString message;
	for (int i=0; i<numclients; i++) {
		message = "LYXSRV:" + clients[i] + ":bye\n";
		pipes.send(message);
	}
}


/* ---F+------------------------------------------------------------------ *\
   Function  : ServerCallback
    Called by : LyXComm
    Purpose   : handle data gotten from communication
\* ---F------------------------------------------------------------------- */

void LyXServer::callback(LyXServer * serv, LString const & msg)
{
	lyxerr.debug("LyXServer: Received: '" + msg + '\'', Error::LYXSERVER);
 
	char const *p = msg.c_str();
 
	// --- parse the string --------------------------------------------
	//
	//  Format: LYXCMD:<client>:<func>:<argstring>\n
	//
	bool server_only = false;
	while(*p) {
		// --- 1. check 'header' ---
	        if (strncmp(p, "LYXSRV:", 7)==0) {
			server_only = true; 
		} else if(0!=strncmp(p, "LYXCMD:", 7)) {
			lyxerr.print("LyXServer: Unknown request");
			return;
		}
		p += 7;
		
		// --- 2. for the moment ignore the client name ---
		LString client;
		while(*p && *p != ':')
			client += char(*p++);
		if(*p == ':') p++;
		if(!*p) return;
		
		// --- 3. get function name ---
		LString cmd;
		while(*p && *p != ':')
			cmd += char(*p++);
		
		// --- 4. parse the argument ---
		LString arg;
		if(!server_only && *p == ':' && *(++p)) {
			while(*p && *p != '\n')
				arg += char(*p++);
			if(*p) p++;
		}
 
		lyxerr.debug("LyXServer: Client: '" + client + "' Command: '" + cmd + "' Argument: '" + arg + '\'', Error::LYXSERVER);
		
		// --- lookup and exec the command ------------------
 
		if (server_only) {
			LString buf;
			// return the greeting to inform the client that 
			// we are listening.
			if (cmd == "hello") {
				// One more client
				if(serv->numclients==MAX_CLIENTS){ //paranoid check
					lyxerr.debug("LyXServer: too many clients...", Error::LYXSERVER);
					return;
				}
				int i=0; //find place in clients[]
				while (!serv->clients[i].empty() 
				       && i<serv->numclients) 
					i++;
				serv->clients[i] = client;
				serv->numclients++;
				buf = "LYXSRV:" + client + ":hello\n";
				lyxerr.debug("LyXServer: Greeting " + client, Error::LYXSERVER);
				serv->pipes.send(buf);
			} else if (cmd == "bye") {
				// If clients==0 maybe we should reset the pipes
				// to prevent fake callbacks
				int i; //look if client is registered
				for (i=0; i<serv->numclients; i++) {
					if (serv->clients[i] == client) break;
				}
				if (i<serv->numclients) {
					serv->numclients--;
					serv->clients[i].clean();
					lyxerr.debug("LyXServer: Client " + client + " said goodbye",
						Error::LYXSERVER);
				} else {
					lyxerr.debug("LyXServer: ignoring bye messge from unregistered client" +
						client + "\n", Error::LYXSERVER);
				}
			} else {
				lyxerr.print("LyXServer: Undefined server command " + cmd + ".");
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

			int action = lyxaction.LookupFunc(cmd.c_str());
			//int action = -1;
			LString rval, buf;
		    
			if (action>=0) {
				rval = serv->func->Dispatch(action, arg.c_str());
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
			buf += LString(client) + ":" + cmd 	+ ":" + rval + "\n";
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

void LyXServer::notifyClient(LString const & s)
{
	LString buf = LString("NOTIFY:") + s + "\n";
	pipes.send(buf);
}

