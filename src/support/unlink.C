#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "lyxlib.h"

int lyx::unlink(string const & pathname)
{
	return ::unlink(pathname.c_str());
}
