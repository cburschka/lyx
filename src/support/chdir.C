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

#include "support/filename.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef _WIN32
# include <windows.h>
#endif


namespace lyx {

int support::chdir(FileName const & name)
{
#ifdef _WIN32
	return SetCurrentDirectory(name.toFilesystemEncoding().c_str()) != 0 ? 0 : -1;
#else
	return ::chdir(name.toFilesystemEncoding().c_str());
#endif
}


} // namespace lyx
