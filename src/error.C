#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "error.h"
#include <cstdio>

Error::Error(int level) // should loglevel also be an argument?
{
	debuglevel = level;
	loglevel = 0;
}


void Error::setDebugLevel(int level)
{
	debuglevel = level;
}


void Error::debug(string const & msg, int level)
{
	if (debuglevel & level)
		print(msg);
	// should also print to the logfile
}

void Error::print(string const & msg)
{
	if (!msg.empty()) 
		fprintf(stderr, "%s\n", msg.c_str());
}


