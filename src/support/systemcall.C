/**
 *  \file systemcall.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * Class Systemcall uses "system" to launch the child process.
 * The user can choose to wait or not wait for the process to complete, but no
 * callback is invoked upon completion of the child.
 *
 * The child process is not killed when the Systemcall instance goes out of
 * scope.
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "systemcall.h"
#include "os.h"

#include <cstdlib> //for ::system

#ifndef CXX_GLOBAL_CSTD
using std::system;
#endif

#if 0
Systemcall::Systemcall(Starttype how, string const & what)
{
	startscript(how, what);
}
#endif


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
