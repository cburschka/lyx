/**
 * \file rmdir.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"

#include <unistd.h>

int lyx::support::rmdir(std::string const & dir)
{
	return ::rmdir(dir.c_str());
}
