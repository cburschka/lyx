#include <config.h>

#include <unistd.h>

#include "support/lyxlib.h"

int lyx::chdir(char const * name)
{
#ifndef __EMX__
	return ::chdir(name);
#else
	return ::_chdir2(name);
#endif
}

int lyx::chdir(string const & name)
{
#ifndef __EMX__
	return ::chdir(name.c_str());
#else
	return ::_chdir2(name.c_str());
#endif
}
