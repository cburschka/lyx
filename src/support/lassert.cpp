/**
 * \file assert.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/debug.h"

#include <boost/assert.hpp>

namespace lyx {

void doAssert(char const * expr,  char const * file, long line)
{
	LYXERR0("ASSERTION " << expr << " VIOLATED IN " << file << ":" << line);
	// comment this out if not needed
	BOOST_ASSERT(false);
}

} // namespace lyx
