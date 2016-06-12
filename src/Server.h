// -*- C++ -*-
/**
 * \file Server.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SERVER_H
#define SERVER_H

#include <boost/signals2/trackable.hpp>

#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <QObject>
#include <QEvent>
#endif


namespace lyx {

class Server;


/** This class managed the pipes used for communicating with clients.
 Usage: Initialize with pipe-filename-base, client class to receive
 messages, and callback-function that will be called with the messages.
 When you want to send, use "send()".
 This class encapsulates all the dirty communication and thus provides
 a clean string interface.
 */
#ifndef _WIN32
class LyXComm : public boost::signals2::trackable {
#else
class LyXComm : public QObject {
	Q_OBJECT

	friend DWORD WINAPI pipeServerWrapper(void *);

public:
	/// Max number of clients
	enum { MAX_CLIENTS = 10 };

private:
	/// Max number of pipe instances
	enum { MAX_PIPES = 2 * MAX_CLIENTS };

	/// I/O buffer size
	enum { PIPE_BUFSIZE = 512 };

	/// Pipe client time-out
	enum { PIPE_TIMEOUT = 5000 };

	/// Pipe states
	enum PipeState {
		CONNECTING_STATE,
		READING_STATE,
		WRITING_STATE
	};

	/// Pipe instances
	typedef struct {
		OVERLAPPED overlap;
		HANDLE handle;
		std::string iobuf;
		char readbuf[PIPE_BUFSIZE];
		DWORD nbytes;
		PipeState state;
		bool pending_io;
	} PipeInst;
#endif
public:
	/** When we receive a message, we send it to a client.
	  This is one of the small things that would have been a lot
	  cleaner with a Signal/Slot thing.
	 */
	typedef void (*ClientCallbackfct)(Server *, std::string const &);

	/// Construct with pipe-basename and callback to receive messages
	LyXComm(std::string const & pip, Server * cli, ClientCallbackfct ccb = 0);

	///
	~LyXComm() { closeConnection(); }

	/// clean up in emergency
	void emergencyCleanup();

	/// Send message
	void send(std::string const &);

	/// asynch ready-to-be-read notification
#ifndef _WIN32
	void read_ready();
#else
	void read_ready(DWORD);
#endif

	/// Tell whether we asked another instance of LyX to open the files
	bool deferredLoading() { return deferred_loading_; }

private:
	/// the filename of the in pipe
	std::string const inPipeName() const;

	/// the filename of the out pipe
	std::string const outPipeName() const;

	/// Open pipes
	void openConnection();

	/// Close pipes
	void closeConnection();

	/// Load files in another running instance of LyX
	bool loadFilesInOtherInstance();

#ifndef _WIN32
	/// start a pipe
	int startPipe(std::string const &, bool);

	/// finish a pipe
	void endPipe(int &, std::string const &, bool);

	/// This is -1 if not open
	int infd_;

	/// This is -1 if not open
	int outfd_;
#else
	/// The pipe server returns false when exiting due to an error
	bool pipeServer();

	/// Start an overlapped connection
	bool startPipe(DWORD);

	/// Reset an overlapped connection
	bool resetPipe(DWORD, bool close_handle = false);

	/// Close event and pipe handles
	void closeHandles();

	/// Catch pipe ready-to-be-read notification
	bool event(QEvent *);

	/// Check whether the pipe server must be stopped
	bool checkStopServer(DWORD timeout = 0);

	/// The filename of a (in or out) pipe instance
	std::string const pipeName(DWORD) const;

	/// Pipe instances
	PipeInst pipe_[MAX_PIPES];

	/// Pipe server control events
	HANDLE event_[MAX_PIPES + 1];

	/// Reply buffer
	std::string outbuf_;

	/// Synchronize access to outbuf_
	HANDLE outbuf_mutex_;

	/// Windows event for stopping the pipe server
	HANDLE stopserver_;

	/// Pipe server thread handle
	HANDLE server_thread_;
#endif

	/// Are we up and running?
	bool ready_;

	/// Base of pipename including path
	std::string pipename_;

	/// The client
	Server * client_;

	/// The client callback function
	ClientCallbackfct clientcb_;

	/// Did we defer loading of files to another instance?
	bool deferred_loading_;
};


///
class Server {
public:
	// FIXME IN 0.13
	// Hack! This should be changed in 0.13

	// IMO lyxserver is atypical, and for the moment the only one, non-gui
	// bufferview. We just have to find a way to handle situations like if
	// lyxserver is using a buffer that is being edited with a bufferview.
	// With a common buffer list this is not a problem, maybe. (Alejandro)
	///
	Server(std::string const & pip);
	///
	~Server();
	///
	void notifyClient(std::string const &);
	///
	bool deferredLoadingToOtherInstance() { return pipes_.deferredLoading(); }

	/// whilst crashing etc.
	void emergencyCleanup() { pipes_.emergencyCleanup(); }
	///
	void callback(std::string const & msg);

private:
	/// Names and number of current clients
#ifndef _WIN32
	enum { MAX_CLIENTS = 10 };
#else
	enum { MAX_CLIENTS = LyXComm::MAX_CLIENTS };
#endif
	///
	std::string clients_[MAX_CLIENTS];
	///
	int numclients_;
	///
	LyXComm pipes_;
};

/// Implementation is in LyX.cpp
Server & theServer();

/// Implementation is in LyX.cpp
extern std::vector<std::string> & theFilesToLoad();


} // namespace lyx

#endif // SERVER_H
