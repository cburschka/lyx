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

#include "LyX.h"

#include "support/debug.h"
#include "support/lyxlib.h"

#include <boost/assert.hpp>

#include <exception>
#include <iomanip>

using namespace std;
using lyx::lyxerr;
using lyx::LyX;

namespace boost {

#ifndef BOOST_NO_EXCEPTIONS
void throw_exception(exception const & e)
{
	lyxerr << "Exception caught:\n" << e.what() << endl;
	BOOST_ASSERT(false);
}
#endif


void emergencyCleanup()
{
	static bool didCleanup;
	if (didCleanup)
		return;

	didCleanup = true;

	LyX::cref().emergencyCleanup();
}


void assertion_failed(char const * expr, char const * function,
		      char const * file, long line)
{
	lyxerr << "Assertion triggered in " << function
	       << " by failing check \"" << expr << "\""
	       << " in file " << file << ":" << line << endl;
	emergencyCleanup();
	lyx::support::abort();
}

} // namespace boost
