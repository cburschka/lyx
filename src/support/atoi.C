#include <config.h>

#include <cstdlib>

#include "lyxlib.h"


int lyx::atoi(string const & nstr)
{
	return ::atoi(nstr.c_str());
}
