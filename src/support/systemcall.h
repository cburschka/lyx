// -*- C++ -*-
/**
 *  \file systemcall.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * An instance of Class Systemcall represents a single child process.
 *
 * Class Systemcall uses system() to launch the child process.
 * The user can choose to wait or not wait for the process to complete, but no
 * callback is invoked upon completion of the child.
 *
 * The child process is not killed when the Systemcall instance goes out of
 * scope.
 */

#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class Systemcall {
public:
	///
	enum Starttype {
		///
		Wait,
		///
		DontWait
	};
	
#if 0
	///
	Systemcall() {}

	/** Generate instance and start child process.
	 *  The string "what" contains a commandline with arguments separated 
	 *  by spaces.
	 */
	Systemcall(Starttype how, string const & what);
#endif
	
	/** Start child process.
	 *  The string "what" contains a commandline with arguments separated 
	 *  by spaces.
	 */
	int startscript(Starttype how, string const & what);
};

#endif // SYSTEMCALL_H
