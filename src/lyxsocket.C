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

#include <iostream>
#include <cerrno>


using std::auto_ptr;
using std::endl;
using std::string;


// Address is the unix address for the socket.
// MAX_CLIENTS is the maximum number of clients
// that can connect at the same time.
LyXServerSocket::LyXServerSocket(LyXFunc * f, string const & addr)
	: func(f),
	  fd_(lyx::support::socktools::listen(addr, MAX_CLIENTS)),
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

	lyx_gui::set_serversocket_callback(this);
	lyxerr[Debug::LYXSERVER] << "lyx: New server socket "
				 << fd_ << ' ' << address_ << endl;
}


// Close the socket and remove the address of the filesystem.
LyXServerSocket::~LyXServerSocket()
{
	::close(fd_);
	lyx::support::unlink(address_);
	while (!clients.empty()) close(*clients.rbegin());
	lyxerr[Debug::LYXSERVER] << "lyx: Server socket quitting" << endl;
}


int LyXServerSocket::fd() const
{
	return fd_;
}


string const & LyXServerSocket::address() const
{
	return address_;
}


// Creates a new LyXDataSocket and checks to see if the connection
// is OK and if the number of clients does not exceed MAX_CLIENTS
void LyXServerSocket::serverCallback()
{
	auto_ptr<LyXDataSocket> client(new LyXDataSocket(this));
	if (client->connected()) {
		if (clients.size() == MAX_CLIENTS) {
			client->writeln("BYE:Too many clients connected");
		} else {
			lyx_gui::set_datasocket_callback(client.get());
			clients.insert(client.release());
			return;
		}
	}
}


// Reads and processes input from client and check
// if the connection has been closed
void LyXServerSocket::dataCallback(LyXDataSocket * client)
{
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
		close(client);
	}
}


// Removes client callback and deletes client object
void LyXServerSocket::close(LyXDataSocket * client)
{
	lyx_gui::remove_datasocket_callback(client);
	clients.erase(client);
	delete client;
}

// Debug
// void LyXServerSocket::dump() const
// {
//	lyxerr << "LyXServerSocket debug dump.\n"
//	     << "fd = " << fd_ << ", address = " << address_ << ".\n"
//	     << "Clients: " << clients.size() << ".\n";
//	if (!clients.empty()) {
//		std::set<LyXDataSocket *>::const_iterator client = clients.begin();
//		std::set<LyXDataSocket *>::const_iterator end = clients.end();
//		for (; client != end; ++client)
//			lyxerr << "fd = " << (*client)->fd() << "\n";
//	}
// }


LyXDataSocket::LyXDataSocket(LyXServerSocket * serv)
	:server_(serv),
	 fd_(lyx::support::socktools::accept(serv->fd()))
{
	if (fd_ == -1) {
		connected_ = false;
	} else {
		lyxerr[Debug::LYXSERVER] << "lyx: New data socket " << fd_ << endl;
		connected_ = true;
	}
}


LyXDataSocket::~LyXDataSocket()
{
	::close(fd_);
	lyxerr[Debug::LYXSERVER] << "lyx: Data socket " << fd_ << " quitting." << endl;
}


LyXServerSocket * LyXDataSocket::server() const
{
	return server_;
}


int LyXDataSocket::fd() const
{
	return fd_;
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
	string::size_type pos;

	// read and store characters in buffer
	while ((count = ::read(fd_, charbuf, charbuf_size - 1)) > 0) {
		charbuf[count] = '\0'; // turn it into a c string
		buffer += charbuf;
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
	if ((pos = buffer.find('\n')) == string::npos) {
		lyxerr[Debug::LYXSERVER] << "lyx: Data socket " << fd_
					 << ": line not completed." << endl;
		return false; // No complete line stored
	}
	line = buffer.substr(0, pos);
	buffer = buffer.substr(pos + 1);
	return true;
}


// Write a line of the form <key>:<value> to the socket
void LyXDataSocket::writeln(string const & line)
{
	string linen(line + '\n');
	int size = linen.size();
	int written = ::write(fd_, linen.c_str(), size);
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
