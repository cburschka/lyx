/**
 * \file abort.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"


namespace lyx {

#include <cstdlib>

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif


void lyx::support::abort()
{
	::abort();
}


} // namespace lyx
