#include <config.h>

#include <unistd.h>

#include "support/lyxlib.h"

char * lyx::getcwd(char * buffer, size_t size)
{
#ifndef __EMX__
	return ::getcwd(buffer, size);
#else
	return ::_getcwd2(buffer, size);
#endif
}
