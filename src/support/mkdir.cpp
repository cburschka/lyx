/**
 * \file mkdir.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lyxlib.h"
#include "support/FileName.h"

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif
#ifdef _WIN32
# include <windows.h>
#endif

namespace lyx {
namespace support {


int mkdir(FileName const & pathname, unsigned long int mode)
{
	// FIXME: why don't we have mode_t in lyx::mkdir prototype ??
#if HAVE_MKDIR
# if MKDIR_TAKES_ONE_ARG
	// MinGW32
	return ::mkdir(pathname.toFilesystemEncoding().c_str());
	// FIXME: "Permissions of created directories are ignored on this system."
# else
	// POSIX
	return ::mkdir(pathname.toFilesystemEncoding().c_str(), mode_t(mode));
# endif
#elif defined(_WIN32)
	// plain Windows 32
	return CreateDirectory(pathname.toFilesystemEncoding().c_str(), 0) != 0 ? 0 : -1;
	// FIXME: "Permissions of created directories are ignored on this system."
#elif HAVE__MKDIR
	return ::_mkdir(pathname.toFilesystemEncoding().c_str());
	// FIXME: "Permissions of created directories are ignored on this system."
#else
#   error "Don't know how to create a directory on this system."
#endif
}


} // namespace support
} // namespace lyx
