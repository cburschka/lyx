/**
 * \file boost.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyx_main.h"
#include "debug.h"
#include "support/lyxlib.h"

#include <boost/assert.hpp>

#include <exception>

using std::endl;

namespace boost {

void throw_exception(std::exception const & e)
{
	lyxerr << "Exception caught:\n"
	    << e.what() << endl;
	BOOST_ASSERT(false);
}


namespace {

void emergencyCleanup()
{
	static bool didCleanup;
	if (didCleanup)
		return;

	didCleanup = true;

	LyX::cref().emergencyCleanup();
}

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


}
