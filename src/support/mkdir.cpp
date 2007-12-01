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
#include "support/debug.h"
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


int mymkdir(char const * pathname, unsigned long int mode)
{
	LYXERR0("MKDIR" << pathname);
	// FIXME: why don't we have mode_t in lyx::mkdir prototype ??
#if HAVE_MKDIR
# if MKDIR_TAKES_ONE_ARG
	// MinGW32
	return ::mkdir(pathname);
	// FIXME: "Permissions of created directories are ignored on this system."
# else
	// POSIX
	return ::mkdir(pathname, mode_t(mode));
# endif
#elif defined(_WIN32)
	// plain Windows 32
	return CreateDirectory(pathname, 0) != 0 ? 0 : -1;
	// FIXME: "Permissions of created directories are ignored on this system."
#elif HAVE__MKDIR
	return ::_mkdir(pathname);
	// FIXME: "Permissions of created directories are ignored on this system."
#else
#   error "Don't know how to create a directory on this system."
#endif

}

int mkdir(FileName const & pathname, unsigned long int mode)
{
	return mymkdir(pathname.toFilesystemEncoding().c_str(), mode);
}


// adapted from zlib-1.2.3/contrib/minizip/miniunz.c
int makedir(char * newdir, unsigned long int mode)
{
	char *buffer;
	char *p;
	int	len = (int)strlen(newdir);

	if (len <= 0)
		return 1;

	buffer = (char*)malloc(len+1);
	strcpy(buffer,newdir);

	if (buffer[len-1] == '/')
		buffer[len-1] = '\0';
	if (mymkdir(buffer, mode) == 0) {
		free(buffer);
		return 0;
	}

	p = buffer + 1;
	while (1) {
		char hold;

		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if (mymkdir(buffer, mode) != 0) {
			free(buffer);
			return 1;
		}
		if (hold == 0)
			break;
		*p++ = hold;
	}
	free(buffer);
	return 0;
}


} // namespace support
} // namespace lyx
