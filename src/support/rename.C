#include <config.h>

#include <cstdio>

#include "support/lyxlib.h"

bool lyx::rename(string const & from, string const & to)
{
#ifdef __EMX__
	lyx::unlink(to.c_str());
#endif
	if (::rename(from.c_str(), to.c_str()) == -1)
		if (lyx::copy(from, to)) {
			lyx::unlink(from);
			return true;
		} else
			return false;
	return true;
}
