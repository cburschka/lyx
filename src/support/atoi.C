#include <config.h>

#include <cstdlib>

#include "lyxlib.h"

#ifndef CXX_GLOBAL_CSTD
using std::atoi;
#endif

int lyx::atoi(string const & nstr)
{
	return ::atoi(nstr.c_str());
}
