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

#include <cstdlib>
#include <iostream>
#include <boost/assert.hpp>

using namespace std;

namespace boost {

#ifndef BOOST_NO_EXCEPTIONS
void throw_exception(exception const & /*e*/)
{
	cerr<<"ASSERTION VIOLATED IN biblio test";
	BOOST_ASSERT(false);
}
#endif


void assertion_failed(char const * /*expr*/, char const * /*function*/,
		      char const * /*file*/, long /*line*/)
{
	::abort();
}

} // namespace boost
