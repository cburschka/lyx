#include <config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef CXX_WORKING_NAMESPACES
namespace lyx {
	int mkdir(char const * pathname, unsigned long int mode)
	{
		return ::mkdir(pathname, mode);
	}
}
#else
#include "lyxlib.h"

int lyx::mkdir(char const * pathname, unsigned long int mode)
{
	return ::mkdir(pathname, mode);
}
#endif
