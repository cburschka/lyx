#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "error.h"
#include <stdio.h>

// 	$Id: error.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: error.C,v 1.1 1999/09/27 18:44:37 larsbj Exp $";
#endif /* lint */

Error::Error(int level) // should loglevel also be an argument?
{
	debuglevel = level;
	loglevel = 0;
}


void Error::setDebugLevel(int level)
{
	debuglevel = level;
}


void Error::debug(LString const & msg, int level)
{
	if (debuglevel & level)
		print(msg);
	// should also print to the logfile
}

void Error::print(LString const & msg)
{
	if (!msg.empty()) 
		fprintf(stderr, "%s\n", msg.c_str());
}


