#include <config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "LString.h"

#include "lyxlib.h"

int lyx::mkdir(string const & pathname, unsigned long int mode)
{
	return ::mkdir(pathname.c_str(), mode_t(mode));
}
