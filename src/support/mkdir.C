#include <config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "LString.h"

#include "lyxlib.h"

int lyx::mkdir(string const & pathname, unsigned long int mode)
{
	// FIXME: why don't we have mode_t in lyx::mkdir prototype ??

#if HAVE_MKDIR
# if MKDIR_TAKES_ONE_ARG
	// MinGW32
	return ::mkdir(pathname.c_str());
# else
	// POSIX
	return ::mkdir(pathname.c_str(), mode_t(mode));
# endif
#else
# if HAVE__MKDIR
	// plain Windows 32
	return ::_mkdir(pathname.c_str());
# else
#  error "Don't know how to create a directory on this system."
# endif
#endif
}
