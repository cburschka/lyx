/**
 * \file chdir.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include <unistd.h>

#include "support/lyxlib.h"

int lyx::chdir(string const & name)
{
#ifndef __EMX__
	return ::chdir(name.c_str());
#else
	return ::_chdir2(name.c_str());
#endif
}
