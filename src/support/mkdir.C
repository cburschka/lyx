/**
 * \file mkdir.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int lyx::support::mkdir(std::string const & pathname, unsigned long int mode)
{
	// FIXME: why don't we have mode_t in lyx::mkdir prototype ??
	return ::mkdir(pathname.c_str(), mode_t(mode));
}
