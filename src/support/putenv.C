#include <config.h>

#include <cstdlib>

#ifndef CXX_GLOBAL_CSTD
using std::putenv;
#endif

#include "lyxlib.h"
int lyx::putenv(char const * str)
{
	return ::putenv(const_cast<char*>(str));
}
