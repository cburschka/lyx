/**
 * \file socktools.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/socktools.h"
#include "support/FileName.h"

#if !defined (HAVE_FCNTL)
// We provide stubs because we don't (yet?) support the native OS API.

namespace lyx {
namespace support {
namespace socktools {

int listen(FileName const &, int)
{
	return -1;
}


int accept(int)
{
	return -1;
}

} // namespace socktools
} // namespace support
} // namespace lyx

#else // defined (HAVE_FCNTL)

#include "support/debug.h"

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <cerrno>
#include <string>
#include <cstring>

using namespace std;


// This MACRO eppears to be defined only on Linux.
#if !defined(SUN_LEN)
#define SUN_LEN(su) \
	(sizeof (*(su)) - sizeof ((su)->sun_path) + strlen((su)->sun_path))
#endif


namespace lyx {
namespace support {
namespace socktools {

// Returns a local socket already in the "listen" state (or -1 in case
// of error). The first argument is the socket address, the second
// is the length of the queue for connections. If successful, a socket
// special file 'name' will be created in the filesystem.
int listen(FileName const & name, int queue)
{
	int fd; // File descriptor for the socket
	sockaddr_un addr; // Structure that hold the socket address

	// We use 'localname' to fill 'addr'
	string const localname = name.toFilesystemEncoding();
	string::size_type len = localname.size();
	// the field sun_path in sockaddr_un is a char[108]
	if (len > 107) {
		LYXERR0("lyx: Socket address '" << name.absFilename() << "' too long.");
		return -1;
	}
	// Synonims for AF_UNIX are AF_LOCAL and AF_FILE
	addr.sun_family = AF_UNIX;
	localname.copy(addr.sun_path, 107);
	addr.sun_path[len] = '\0';

	// This creates a file descriptor for the socket
	// Synonims for PF_UNIX are PF_LOCAL and PF_FILE
	// For local sockets, the protocol is always 0
	// socket() returns -1 in case of error
	if ((fd = ::socket(PF_UNIX, SOCK_STREAM, 0))== -1) {
		LYXERR0("lyx: Could not create socket descriptor: "
		       << strerror(errno));
		return -1;
	}

	// Set NONBLOCK mode for the file descriptor
	if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		LYXERR0("lyx: Could not set NONBLOCK mode for socket descriptor: "
		     << strerror(errno));
		::close(fd);
		return -1;
	}

	// bind() gives the local address 'name' for 'fd', also creating
	// the socket special file in the filesystem. bind() returns -1
	// in case of error
	if ((::bind (fd, reinterpret_cast<sockaddr *>(&addr), SUN_LEN(&addr))) == -1) {
		LYXERR0("lyx: Could not bind address '" << name.absFilename()
		       << "' to socket descriptor: " << strerror(errno));
		::close(fd);
		name.removeFile();
		return -1;
	}

	// Puts the socket in listen state, that is, ready to accept
	// connections. The second parameter of listen() defines the
	// maximum length the queue of pending connections may grow to.
	// It is not a restriction on the number of connections the socket
	// can accept. Returns -1 in case of error
	if (::listen (fd, queue) == -1) {
		LYXERR0("lyx: Could not put socket in 'listen' state: "
		       << strerror(errno));
		::close(fd);
		name.removeFile();
		return -1;
	}

	return fd;
}

// Returns a file descriptor for a new connection from the socket
// descriptor 'sd' (or -1 in case of error)
int accept(int sd)
{
	int fd;

	// Returns the new file descriptor or -1 in case of error
	// Using null pointers for the second and third arguments
	// dismiss all information about the connecting client
	if ((fd = accept(sd, reinterpret_cast<sockaddr *>(0), reinterpret_cast<socklen_t *>(0))) == -1) {
		LYXERR0("lyx: Could not accept connection: "
		       << strerror(errno));
		return -1;
	}

	// Sets NONBLOCK mode for the file descriptor
	if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		LYXERR0("lyx: Could not set NONBLOCK mode for connection: "
		       << strerror(errno));
		::close(fd);
		return -1;
	}
	return fd;
}

} // namespace socktools
} // namespace support
} // namespace lyx

#endif // defined (HAVE_FCNTL)
