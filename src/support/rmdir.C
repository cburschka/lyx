#include <config.h>

#include <unistd.h>

#include "lyxlib.h"

int lyx::rmdir(string const & dir)
{
	return ::rmdir(dir.c_str());
}
