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

#include "debug.h"
#include "support/LAssert.h"

#include <exception>

using namespace lyx::support;

using std::endl;

namespace boost {

void throw_exception(std::exception const & e)
{
	lyxerr << "Exception caught:\n"
	    << e.what() << endl;
	Assert(false);
}


}
