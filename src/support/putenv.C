#include <config.h>

#include <stdlib.h>

#include "lyxlib.h"
int lyx::putenv(char const * str)
{
	return ::putenv(const_cast<char*>(str));
}
