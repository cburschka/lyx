/**
 * \file chdir.C
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

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef _WIN32
# include <windows.h>
#endif

int lyx::support::chdir(std::string const & name)
{
#ifdef _WIN32
	return SetCurrentDirectory(name.c_str()) != 0 ? 0 : -1;
#else
	return ::chdir(name.c_str());
#endif
}


} // namespace lyx
