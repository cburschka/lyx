#include <config.h>

#include "lyxlib.h"

#include <cstdlib>

int lyx::putenv(char const * str)
{
	return ::putenv(const_cast<char*>(str));
}
