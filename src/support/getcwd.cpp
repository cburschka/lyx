/**
 * \file getcwd.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"
#include "support/docstring.h"
#include "support/os.h"

#include <boost/scoped_array.hpp>

#include <cerrno>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef _WIN32
# include <windows.h>
#endif

using boost::scoped_array;

using std::string;


namespace lyx {
namespace support {

namespace {

inline
char * l_getcwd(char * buffer, size_t size)
{
#ifdef _WIN32
	GetCurrentDirectory(size, buffer);
	return buffer;
#else
	return ::getcwd(buffer, size);
#endif
}

} // namespace anon


// Returns current working directory
FileName const getcwd()
{
	int n = 256;	// Assume path is less than 256 chars
	char * err;
	scoped_array<char> tbuf(new char[n]);

	// Safe. Hopefully all getcwds behave this way!
	while (((err = l_getcwd(tbuf.get(), n)) == 0) && (errno == ERANGE)) {
		// Buffer too small, double the buffersize and try again
		n *= 2;
		tbuf.reset(new char[n]);
	}

	string result;
	if (err)
		result = tbuf.get();
	return FileName(os::internal_path(to_utf8(from_filesystem8bit(result))));
}

} // namespace support
} // namespace lyx
