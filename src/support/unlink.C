#include <config.h>

#include <unistd.h>

#include "lyxlib.h"

int lyx::unlink(string const & pathname)
{
	return ::unlink(pathname.c_str());
}
