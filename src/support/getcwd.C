/**
 * \file getcwd.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#include <boost/scoped_array.hpp>

#include <cerrno>
#include <unistd.h>

using boost::scoped_array;

using std::string;


namespace {

inline
char * l_getcwd(char * buffer, size_t size)
{
#ifndef __EMX__
	return ::getcwd(buffer, size);
#else
	return ::_getcwd2(buffer, size);
#endif
}

} // namespace anon


// Returns current working directory
string const lyx::support::getcwd()
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
	return result;
}
