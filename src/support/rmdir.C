#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "lyxlib.h"

int lyx::rmdir(string const & dir)
{
	return ::rmdir(dir.c_str());
}
