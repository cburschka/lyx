/**
 * \file atoi.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#include <cstdlib>

#ifndef CXX_GLOBAL_CSTD
using std::atoi;
#endif

int lyx::support::atoi(std::string const & nstr)
{
	return ::atoi(nstr.c_str());
}
