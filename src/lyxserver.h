// -*- C++ -*-
/* This file is part of
* ====================================================== 
* 
*           LyX, The Document Processor
*        
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2000 The LyX Team.
*
* ====================================================== */

#ifndef LYXSERVER_H
#define LYXSERVER_H

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
 a clean string interface.
 */
class LyXComm
{
public:
	/** When we receive a message, we send it to a client.
	  This is one of the small things that would have been a lot
	  cleaner with a Signal/Slot thing.
	 */
	typedef void (*ClientCallbackfct)(LyXServer *, string const &);

	/// Construct with pipe-basename and callback to receive messages
	LyXComm(string const & pip, LyXServer * cli, ClientCallbackfct ccb = 0)
		: pipename(pip), client(cli), clientcb(ccb)
	{
		ready = false;
		openConnection();
	}

	///
	~LyXComm() {
		closeConnection();
	}

	/// Send message
	void send(string const &);

	/// We receive messages via XForms through this callback
	static void callback(int fd, void *v);

private:
	/// Open pipes
	void openConnection();
	
	/// Close pipes
	void closeConnection();

	/// This is -1 if not open
	int infd;

	/// This is -1 if not open
	int outfd;

	/// Are we up and running?
	bool ready;

	/// Base of pipename including path
	string pipename;

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
	LyXServer(LyXFunc * f, string const & pip)
		: numclients(0), func(f), pipes(pip, (this), callback)
	{ }
        /// 
	~LyXServer();
	///
	void notifyClient(string const &);
private:
	///
	static void callback(LyXServer *, string const & msg);
	/// Names and number of current clients
	enum { MAX_CLIENTS = 10 };
	string clients[MAX_CLIENTS];
	int numclients;
	///
	LyXFunc * func;
	///
	LyXComm pipes;
};

#endif /* _LYXSERVER_H_ */

/* === End of File: lyxserver.h ========================================== */
