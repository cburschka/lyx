#include <config.h>

#include <stdlib.h>

#ifdef CXX_WORKING_NAMESPACES
namespace lyx {
	int putenv(char const * str)
	{
		return ::putenv(const_cast<char*>(str));
	}
}
#else
#include "lyxlib.h"
int lyx::putenv(char const * str)
{
	return ::putenv(const_cast<char*>(str));
}

#endif
