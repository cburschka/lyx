/**
 * \file rename.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include <cstdio>

#include "support/lyxlib.h"

bool lyx::support::rename(string const & from, string const & to)
{
#ifdef __EMX__
	unlink(to);
#endif
	if (::rename(from.c_str(), to.c_str()) == -1)
		if (copy(from, to)) {
			unlink(from);
			return true;
		} else
			return false;
	return true;
}
