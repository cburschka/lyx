/**
 *  \file systemcall.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "systemcall.h"
#include "os.h"

#include <cstdlib>


// Reuse of instance
int Systemcall::startscript(Starttype how, string const & what)
{
	string command = what;

	if (how == DontWait) {
		if (os::shell() == os::UNIX) {
			command += " &";
		} else {
			command = "start /min/n " + command;
		}
	}

	return ::system(command.c_str());
}
