// -*- C++ -*-
/**
 * \file lyxsocket.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXSOCKET_H
#define LYXSOCKET_H

#include "support/socktools.h"
#include "lyxfunc.h"

#include <string>
#include <set>

class LyXServerSocket;
class LyXDataSocket;

/** Sockets can be in two states: listening and connected.
 *  Connected sockets are used to transfer data, and will therefore
 *  be called Data Sockets. Listening sockets are used to create
 *  Data Sockets when clients connect, and therefore will be called
 * Server Sockets.

 * This class encapsulates local (unix) server socket operations and
 * manages LyXDataSockets objects that are created when clients connect.
 */
class LyXServerSocket
{
public:
	LyXServerSocket(LyXFunc *, std::string const &);
	~LyXServerSocket();
	/// File descriptor of the socket
	int fd() const;
	/// Address of the local socket
	std::string const & address() const;
	/// To be called when there is activity in the server socket
	void serverCallback();
	/// To be called when there is activity in the data socket
	void dataCallback(LyXDataSocket *);

private:
	/// Close the connection to the argument client
	void close(LyXDataSocket *);

	LyXFunc * func;
	/// File descriptor for the server socket
	int fd_;
	/// Stores the socket filename
	std::string address_;
	/// Maximum number of simultaneous clients
	enum {
                MAX_CLIENTS = 10
        };
	/// All connections
	std::set<LyXDataSocket *> clients;
};


/** This class encapsulates data socket operations.
 *  It provides read and write IO operations on the socket.
 */
class LyXDataSocket
{
public:
	LyXDataSocket(LyXServerSocket *);
	~LyXDataSocket();
	/// The object that allocated us
	LyXServerSocket * server() const;
	/// File descriptor of the connection
	int fd() const;
	/// Connection status
	bool connected() const;
	/// Line buffered input from the socket
	bool readln(std::string &);
	/// Write the string + '\n' to the socket
	void writeln(std::string const &);

private:
	LyXServerSocket * server_;
	/// File descriptor for the data socket
	int fd_;
	/// True if the connection is up
	bool connected_;
	/// buffer for input data
	std::string buffer;
};

#endif // LYXSOCKET_H
