/**
 * \file ServerSocket.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ServerSocket.h"

#include "DispatchResult.h"
#include "FuncRequest.h"
#include "LyX.h"
#include "LyXAction.h"

#include "frontends/Application.h"

#include "support/debug.h"
#include "support/environment.h"
#include "support/FileName.h"
#include "support/socktools.h"

#include "support/bind.h"

#include <cerrno>
#include <ostream>

#if defined (_WIN32)
# include <io.h>
#endif

using namespace std;
using namespace lyx::support;


namespace lyx {

// Address is the unix address for the socket.
// MAX_CLIENTS is the maximum number of clients
// that can connect at the same time.
ServerSocket::ServerSocket(FileName const & addr)
	: fd_(socktools::listen(addr, 3)),
	  address_(addr)
{
	if (fd_ == -1) {
		LYXERR(Debug::LYXSERVER, "lyx: Disabling LyX socket.");
		return;
	}

	// These env vars are used by DVI inverse search
	// Needed by xdvi
	setEnv("XEDITOR", "lyxclient -g %f %l");
	// Needed by lyxclient
	setEnv("LYXSOCKET", address_.absFileName());

	theApp()->registerSocketCallback(
		fd_,
		bind(&ServerSocket::serverCallback, this)
		);

	LYXERR(Debug::LYXSERVER, "lyx: New server socket "
				 << fd_ << ' ' << address_.absFileName());
}


// Close the socket and remove the address of the filesystem.
ServerSocket::~ServerSocket()
{
	if (fd_ != -1) {
		BOOST_ASSERT (theApp());
		theApp()->unregisterSocketCallback(fd_);
		if (::close(fd_) != 0)
			lyxerr << "lyx: Server socket " << fd_
			       << " IO error on closing: " << strerror(errno)
			       << endl;
	}
	address_.removeFile();
	LYXERR(Debug::LYXSERVER, "lyx: Server socket quitting");
}


string const ServerSocket::address() const
{
	return address_.absFileName();
}


// Creates a new LyXDataSocket and checks to see if the connection
// is OK and if the number of clients does not exceed MAX_CLIENTS
void ServerSocket::serverCallback()
{
	if (clients.size() >= MAX_CLIENTS) {
		writeln("BYE:Too many clients connected");
		return;
	}

	int const client_fd = socktools::accept(fd_);

	if (fd_ == -1) {
		LYXERR(Debug::LYXSERVER, "lyx: Failed to accept new client");
		return;
	}

	// Register the new client.
	clients[client_fd] =
		shared_ptr<LyXDataSocket>(new LyXDataSocket(client_fd));
	theApp()->registerSocketCallback(
		client_fd,
		bind(&ServerSocket::dataCallback,
			    this, client_fd)
		);
}


// Reads and processes input from client and check
// if the connection has been closed
void ServerSocket::dataCallback(int fd)
{
	map<int, shared_ptr<LyXDataSocket> >::const_iterator it = clients.find(fd);
	if (it == clients.end())
		return;
	shared_ptr<LyXDataSocket> client = it->second;
	string line;
	bool saidbye = false;
	while (!saidbye && client->readln(line)) {
		// The protocol must be programmed here
		// Split the key and the data
		size_t pos;
		if ((pos = line.find(':')) == string::npos) {
			client->writeln("ERROR:" + line + ":malformed message");
			continue;
		}

		string const key = line.substr(0, pos);
		if (key == "LYXCMD") {
			string const cmd = line.substr(pos + 1);
			FuncRequest fr(lyxaction.lookupFunc(cmd));
			fr.setOrigin(FuncRequest::LYXSERVER);
			DispatchResult dr;
			theApp()->dispatch(fr, dr);
			string const rval = to_utf8(dr.message());
			if (dr.error())
				client->writeln("ERROR:" + cmd + ':' + rval);
			else
				client->writeln("INFO:" + cmd + ':' + rval);
		} else if (key == "HELLO") {
			// no use for client name!
			client->writeln("HELLO:");
		} else if (key == "BYE") {
			saidbye = true;
		} else {
			client->writeln("ERROR:unknown key " + key);
		}
	}

	if (saidbye || !client->connected()) {
		clients.erase(fd);
	}
}


void ServerSocket::writeln(string const & line)
{
	string const linen = line + '\n';
	int const size = linen.size();
	int const written = ::write(fd_, linen.c_str(), size);
	if (written < size) { // Always mean end of connection.
		if (written == -1 && errno == EPIPE) {
			// The program will also receive a SIGPIPE
			// that must be caught
			lyxerr << "lyx: Server socket " << fd_
			       << " connection closed while writing." << endl;
		} else {
			// Anything else, including errno == EAGAIN, must be
			// considered IO error. EAGAIN should never happen
			// when line is small
			lyxerr << "lyx: Server socket " << fd_
			     << " IO error: " << strerror(errno);
		}
	}
}

// Debug
// void ServerSocket::dump() const
// {
//	lyxerr << "ServerSocket debug dump.\n"
//	     << "fd = " << fd_ << ", address = " << address_.absFileName() << ".\n"
//	     << "Clients: " << clients.size() << ".\n";
//	map<int, shared_ptr<LyXDataSocket> >::const_iterator client = clients.begin();
//	map<int, shared_ptr<LyXDataSocket> >::const_iterator end = clients.end();
//	for (; client != end; ++client)
//		lyxerr << "fd = " << client->first << '\n';
// }


LyXDataSocket::LyXDataSocket(int fd)
	: fd_(fd), connected_(true)
{
	LYXERR(Debug::LYXSERVER, "lyx: New data socket " << fd_);
}


LyXDataSocket::~LyXDataSocket()
{
	if (::close(fd_) != 0)
		lyxerr << "lyx: Data socket " << fd_
		       << " IO error on closing: " << strerror(errno);

	theApp()->unregisterSocketCallback(fd_);
	LYXERR(Debug::LYXSERVER, "lyx: Data socket " << fd_ << " quitting.");
}


bool LyXDataSocket::connected() const
{
	return connected_;
}


// Returns true if there was a complete line to input
bool LyXDataSocket::readln(string & line)
{
	int const charbuf_size = 100;
	char charbuf[charbuf_size]; // buffer for the ::read() system call
	int count;

	// read and store characters in buffer
	while ((count = ::read(fd_, charbuf, charbuf_size - 1)) > 0) {
		buffer_.append(charbuf, charbuf + count);
	}

	// Error conditions. The buffer must still be
	// processed for lines read
	if (count == 0) { // EOF -- connection closed
		LYXERR(Debug::LYXSERVER, "lyx: Data socket " << fd_
					 << ": connection closed.");
		connected_ = false;
	} else if ((count == -1) && (errno != EAGAIN)) { // IO error
		lyxerr << "lyx: Data socket " << fd_
		       << ": IO error." << endl;
		connected_ = false;
	}

	// Cut a line from buffer
	size_t pos = buffer_.find('\n');
	if (pos == string::npos) {
		LYXERR(Debug::LYXSERVER, "lyx: Data socket " << fd_
					 << ": line not completed.");
		return false; // No complete line stored
	}
	line = buffer_.substr(0, pos);
	buffer_.erase(0, pos + 1);
	return true;
}


// Write a line of the form <key>:<value> to the socket
void LyXDataSocket::writeln(string const & line)
{
	string const linen = line + '\n';
	int const size = linen.size();
	int const written = ::write(fd_, linen.c_str(), size);
	if (written < size) { // Always mean end of connection.
		if (written == -1 && errno == EPIPE) {
			// The program will also receive a SIGPIPE
			// that must be catched
			lyxerr << "lyx: Data socket " << fd_
			       << " connection closed while writing." << endl;
		} else {
			// Anything else, including errno == EAGAIN, must be
			// considered IO error. EAGAIN should never happen
			// when line is small
			lyxerr << "lyx: Data socket " << fd_
			     << " IO error: " << strerror(errno);
		}
		connected_ = false;
	}
}


} // namespace lyx
