#include <config.h>

#include <stdio.h>

#include "support/lyxlib.h"

bool lyx::rename(char const * from, char const * to)
{
	return ::rename(from, to) != -1;
}
