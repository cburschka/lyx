#include <config.h>

#include <cstdlib>

#include "lyxlib.h"
int lyx::putenv(char const * str)
{
	return ::putenv(const_cast<char*>(str));
}
