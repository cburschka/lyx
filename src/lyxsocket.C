/**
 * \file lyxsocket.C
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

#include "lyxsocket.h"

#include "debug.h"
#include "funcrequest.h"
#include "LyXAction.h"
#include "lyxfunc.h"

#include "frontends/lyx_gui.h"

#include "support/lyxlib.h"
#include "support/socktools.h"

#include <boost/bind.hpp>

#include <cerrno>

using boost::shared_ptr;

using std::auto_ptr;
using std::endl;
using std::string;


// Address is the unix address for the socket.
// MAX_CLIENTS is the maximum number of clients
// that can connect at the same time.
LyXServerSocket::LyXServerSocket(LyXFunc * f, string const & addr)
	: func(f),
	  fd_(lyx::support::socktools::listen(addr, 3)),
	  address_(addr)
{
	if (fd_ == -1) {
		lyxerr << "lyx: Disabling LyX socket." << endl;
		return;
	}

	// These env vars are used by DVI inverse search
	// Needed by xdvi
	lyx::support::putenv("XEDITOR", "lyxclient -g %f %l");
	// Needed by lyxclient
	lyx::support::putenv("LYXSOCKET", address_);

	lyx_gui::register_socket_callback(
		fd_,
		boost::bind(&LyXServerSocket::serverCallback, *this)
		);

	lyxerr[Debug::LYXSERVER] << "lyx: New server socket "
				 << fd_ << ' ' << address_ << endl;
}


// Close the socket and remove the address of the filesystem.
LyXServerSocket::~LyXServerSocket()
{
	lyx_gui::unregister_socket_callback(fd_);
	::close(fd_);
	lyx::support::unlink(address_);
	lyxerr[Debug::LYXSERVER] << "lyx: Server socket quitting" << endl;
}


string const & LyXServerSocket::address() const
{
	return address_;
}


// Creates a new LyXDataSocket and checks to see if the connection
// is OK and if the number of clients does not exceed MAX_CLIENTS
void LyXServerSocket::serverCallback()
{
	int const client_fd = lyx::support::socktools::accept(fd_);

	if (fd_ == -1) {
		lyxerr[Debug::LYXSERVER] << "lyx: Failed to accept new client"
					 << endl;
		return;
	}

	if (clients.size() >= MAX_CLIENTS) {
		writeln("BYE:Too many clients connected");
		return;
	}

	// Register the new client.
	clients[client_fd] =
		shared_ptr<LyXDataSocket>(new LyXDataSocket(client_fd));
	lyx_gui::register_socket_callback(
		client_fd,
		boost::bind(&LyXServerSocket::dataCallback,
			    *this, client_fd)
		);
}


// Reads and processes input from client and check
// if the connection has been closed
void LyXServerSocket::dataCallback(int fd)
{
	shared_ptr<LyXDataSocket> client = clients[fd];

	string line;
	string::size_type pos;
	bool saidbye = false;
	while ((!saidbye) && client->readln(line)) {
		// The protocol must be programmed here
		// Split the key and the data
		if ((pos = line.find(':')) == string::npos) {
			client->writeln("ERROR:" + line + ":malformed message");
			continue;
		}

		string const key = line.substr(0, pos);
		if (key == "LYXCMD") {
			string const cmd = line.substr(pos + 1);
			func->dispatch(lyxaction.lookupFunc(cmd));
			string const rval = func->getMessage();
			if (func->errorStat()) {
				client->writeln("ERROR:" + cmd + ':' + rval);
			} else {
				client->writeln("INFO:" + cmd + ':' + rval);
			}
		} else if (key == "HELLO") {
			// no use for client name!
			client->writeln("HELLO:");
		} else if (key == "BYE") {
			saidbye = true;
		} else {
			client->writeln("ERROR:unknown key " + key);
		}
	}

	if (saidbye || (!client->connected())) {
		clients.erase(fd);
	}
}


void LyXServerSocket::writeln(string const & line)
{
	string const linen(line + '\n');
	int const size = linen.size();
	int const written = ::write(fd_, linen.c_str(), size);
	if (written < size) { // Allways mean end of connection.
		if ((written == -1) && (errno == EPIPE)) {
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
// void LyXServerSocket::dump() const
// {
//	lyxerr << "LyXServerSocket debug dump.\n"
//	     << "fd = " << fd_ << ", address = " << address_ << ".\n"
//	     << "Clients: " << clients.size() << ".\n";
//	std::map<int, shared_ptr<LyXDataSocket> >::const_iterator client = clients.begin();
//	std::map<int, shared_ptr<LyXDataSocket> >::const_iterator end = clients.end();
//	for (; client != end; ++client)
//		lyxerr << "fd = " << client->first << '\n';
// }


LyXDataSocket::LyXDataSocket(int fd)
	: fd_(fd), connected_(true)
{
	lyxerr[Debug::LYXSERVER] << "lyx: New data socket " << fd_ << endl;
}


LyXDataSocket::~LyXDataSocket()
{
	::close(fd_);

	lyx_gui::unregister_socket_callback(fd_);
	lyxerr[Debug::LYXSERVER] << "lyx: Data socket " << fd_ << " quitting."
				 << endl;
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
		lyxerr[Debug::LYXSERVER] << "lyx: Data socket " << fd_
					 << ": connection closed." << endl;
		connected_ = false;
	} else if ((count == -1) && (errno != EAGAIN)) { // IO error
		lyxerr << "lyx: Data socket " << fd_
		       << ": IO error." << endl;
		connected_ = false;
	}

	// Cut a line from buffer
	string::size_type pos = buffer_.find('\n');
	if (pos == string::npos) {
		lyxerr[Debug::LYXSERVER] << "lyx: Data socket " << fd_
					 << ": line not completed." << endl;
		return false; // No complete line stored
	}
	line = buffer_.substr(0, pos);
	buffer_.erase(0, pos + 1);
	return true;
}


// Write a line of the form <key>:<value> to the socket
void LyXDataSocket::writeln(string const & line)
{
	string const linen(line + '\n');
	int const size = linen.size();
	int const written = ::write(fd_, linen.c_str(), size);
	if (written < size) { // Allways mean end of connection.
		if ((written == -1) && (errno == EPIPE)) {
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
