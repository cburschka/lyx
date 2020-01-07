/**
 * \file boost.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/debug.h"

#include <boost/assert.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>

using namespace std;
using lyx::lyxerr;

namespace boost {

#ifndef BOOST_NO_EXCEPTIONS
void throw_exception(exception const & e)
{
	lyxerr << "Exception caught:\n" << e.what() << endl;
	BOOST_ASSERT(false);
}
#endif


void assertion_failed(char const * expr, char const * function,
		char const * file, long line)
{
	lyxerr << "Assertion triggered in " << function
	       << " by failing check \"" << expr << "\""
	       << " in file " << file << ":" << line << endl;
	::abort();
}


void assertion_failed_msg(char const * expr, char const * msg,
		char const * function, char const * file, long line)
{
	lyxerr << "Assertion triggered in " << function
	       << " by failing check \"" << expr << "\""
	       << " with message \"" << msg << "\""
	       << " in file " << file << ":" << line << endl;

	::abort();
}

} // namespace boost
