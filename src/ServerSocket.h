// -*- C++ -*-
/**
 * \file ServerSocket.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXSERVERSOCKET_H
#define LYXSERVERSOCKET_H

#include "support/FileName.h"

#include <string>
#include <map>
#include <memory>


namespace lyx {

class LyXDataSocket;


/** Sockets can be in two states: listening and connected.
 *  Connected sockets are used to transfer data, and will therefore
 *  be called Data Sockets. Listening sockets are used to create
 *  Data Sockets when clients connect, and therefore will be called
 * Server Sockets.

 * This class encapsulates local (unix) server socket operations and
 * manages LyXDataSockets objects that are created when clients connect.
 */
class ServerSocket {
public:
	///
	ServerSocket(support::FileName const &);
	///
	~ServerSocket();
	/// Address of the local socket
	std::string const address() const;
	/// To be called when there is activity in the server socket
	void serverCallback();
	/// To be called when there is activity in the data socket
	void dataCallback(int fd);
private:
	///
	void writeln(std::string const &);
	/// File descriptor for the server socket
	int fd_;
	/// Stores the socket filename
	support::FileName address_;
	/// Maximum number of simultaneous clients
	enum {
		MAX_CLIENTS = 10
	};
	/// All connections
	std::map<int, std::shared_ptr<LyXDataSocket>> clients;
};


/** This class encapsulates data socket operations.
 *  It provides read and write IO operations on the socket.
 */
class LyXDataSocket {
public:
	///
	LyXDataSocket(int fd);
	///
	~LyXDataSocket();
	/// Connection status
	bool connected() const;
	/// Line buffered input from the socket
	bool readln(std::string &);
	/// Write the string + '\n' to the socket
	void writeln(std::string const &);
private:
	/// File descriptor for the data socket
	int fd_;
	/// True if the connection is up
	bool connected_;
	/// buffer for input data
	std::string buffer_;
};

/// Implementation is in LyX.cpp
extern ServerSocket & theServerSocket();


} // namespace lyx

#endif // LYXSERVERSOCKET_H
