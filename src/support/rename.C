#include <config.h>

#include <cstdio>

#include "support/lyxlib.h"

bool lyx::rename(string const & from, string const & to)
{
	return ::rename(from.c_str(), to.c_str()) != -1;
}
