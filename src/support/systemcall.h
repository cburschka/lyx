// -*- C++ -*-
/**
 *  \file systemcall.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
 * An instance of Class Systemcall represents a single child process.
 *
 * Class Systemcall uses system() to launch the child process.
 * The user can choose to wait or not wait for the process to complete, but no
 * callback is invoked upon completion of the child.
 *
 * The child process is not killed when the Systemcall instance goes out of
 * scope.
 */
class Systemcall {
public:
	/// whether to wait for completion
	enum Starttype {
		Wait, //< wait for completion before returning from startscript()
		DontWait //< don't wait for completion
	};

	/** Start child process.
	 *  The string "what" contains a commandline with arguments separated
	 *  by spaces.
	 */
	int startscript(Starttype how, string const & what);
};

#endif // SYSTEMCALL_H
