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

#ifndef _LYXSERVER_H_
#define _LYXSERVER_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
class LyXFunc;
class LyXServer;

/* --- i/o pipes --------------------------------------------------------- */

/** This class managed the pipes used for communicating with clients.
 Usage: Initialize with pipe-filename-base, client class to receive
 messages, and callback-function that will be called with the messages.
 When you want to send, use "send()".
 This class encapsulates all the dirty communication and thus provides
 a clean LString interface.
 */
class LyXComm
{
public:
	/** When we receive a message, we send it to a client.
	  This is one of the small things that would have been a lot
	  cleaner with a Signal/Slot thing.
	 */
	typedef void (*ClientCallbackfct)(LyXServer *, LString const &);

	/// Construct with pipe-basename and callback to receive messages
	LyXComm(LString const &pip, LyXServer * cli, ClientCallbackfct ccb = 0)
		:pipename(pip), client(cli), clientcb(ccb)
	{
		ready = false;
		openConnection();
	}

	///
	~LyXComm() {
		closeConnection();
	}

	/// Send message
	void send(LString const &);
private:
	/// Open pipes
	void openConnection();
	
	/// Close pipes
	void closeConnection();

	/// We receive messages via XForms through this callback
	static void callback(int fd, void *v);

	/// This is -1 if not open
	int infd;

	/// This is -1 if not open
	int outfd;

	/// Are we up and running?
	bool ready;

	/// Base of pipename including path
	LString pipename;

	/// The client
	LyXServer * client;

	/// The client callback function
	ClientCallbackfct clientcb;
};


/* --- prototypes -------------------------------------------------------- */
class LyXServer
{
public:
	// FIXME IN 0.13
	// Hack! This should be changed in 0.13

	/// The lyx server should not take an argument "LyXFunc" but this is
	// how it will be done for 0.12. In 0.13 we must write a non-gui
	// bufferview.
        // IMO lyxserver is atypical, and for the moment the only one, non-gui
        // bufferview. We just have to find a way to handle situations like if
        // lyxserver is using a buffer that is being edited with a bufferview.
        // With a common buffer list this is not a problem, maybe. (Alejandro)
	LyXServer(LyXFunc *f, LString const &pip)
		: numclients(0), func(f), pipes(pip, (this), callback)
	{ }
        /// 
	~LyXServer();
	///
	void notifyClient(LString const &);
private:
	///
	static void callback(LyXServer *, LString const & msg);
	/// Names and number of current clients
	enum { MAX_CLIENTS = 10 };
	LString clients[MAX_CLIENTS];
	int numclients;
	///
	LyXFunc *func;
	///
	LyXComm pipes;
};

#endif /* _LYXSERVER_H_ */

/* === End of File: lyxserver.h ========================================== */
