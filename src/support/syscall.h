// -*- C++ -*-
/**
 *  \file syscall.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * Class Systemcalls uses "system" to launch the child process.
 * The user can choose to wait or not wait for the process to complete, but no
 * callback is invoked upon completion of the child.
 *
 * The child process is not killed when the Systemcall instance goes out of
 * scope.
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class Systemcalls {
public:
	///
	enum Starttype {
		///
		Wait,
		///
		DontWait
	};
	
	///
	Systemcalls() {}

#if 0
	/** Generate instance and start child process.
	 *  The string "what" contains a commandline with arguments separated 
	 *  by spaces.
	 */
	Systemcalls(Starttype how, string const & what);
#endif
	
	/** Start child process.
	 *  This is for reuse of the Systemcalls instance.
	 */
	int startscript(Starttype how, string const & what);
};

#endif // SYSCALL_H
