// -*- C++ -*-
/**
 *  \file forkedcall.h
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * An instance of Class Forkedcall represents a single child process.
 *
 * Class Forkedcall uses fork() and execvp() to lauch the child process.
 *
 * Once launched, control is returned immediately to the parent process
 * but a Signal can be emitted upon completion of the child.
 *
 * The child process is not killed when the Forkedcall instance goes out of
 * scope, but it can be killed by an explicit invocation of the kill() member
 * function.
 */

#ifndef FORKEDCALL_H
#define FORKEDCALL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <sys/types.h>
#include <boost/smart_ptr.hpp>
#include <sigc++/signal_system.h>


class Forkedcall {
public:
	///
	enum Starttype {
		///
		Wait,
		///
		DontWait
	};

	///
	Forkedcall();

	/** Start the child process.
	 *
	 *  The command "what" is passed to fork() for execution.
	 *
	 *  There are two startscript commands available. They differ in that
	 *  the second receives a signal that is executed on completion of
	 *  the command. This makes sense only for a command executed
	 *  in the background, ie DontWait.
	 *
	 *  The other startscript command can be executed either blocking
	 *  or non-blocking, but no signal will be emitted on finishing.
	 */
	int startscript(Starttype, string const & what);

	/** A SignalType signal is can be emitted once the forked process
	 *  has finished. It passes:
	 *  the commandline string;
	 *  the PID of the child and;
	 *  the return value from the child.
	 *
	 *  We use a signal rather than simply a callback function so that
	 *  we can return easily to C++ methods, rather than just globally
	 *  accessible functions.
	 */
	typedef SigC::Signal3<void, string const &, pid_t, int> SignalType;

	/** The signal is connected in the calling routine to the desired
	 *  slot. We pass a shared_ptr rather than a reference to the signal
	 *  because it is eminently possible for the instance of the calling
	 *  class (and hence the signal) to be destructed before the forked
	 *  call is complete.
	 *
	 *  It doesn't matter if the slot disappears, SigC takes care of that.
	 */
	typedef boost::shared_ptr<SignalType> SignalTypePtr;

	///
	int startscript(string const & what, SignalTypePtr);

	/** Invoking the following methods makes sense only if the command
	 *  is running asynchronously!
	 */

	/** gets the PID of the child process.
	 *  Used by the timer.
	 */
	pid_t pid() const { return pid_; }

	/** Emit the signal.
	 *  Used by the timer.
	 */
	void emitSignal();

	/** Set the return value of the child process.
	 *  Used by the timer.
	 */
	void setRetValue(int r) { retval_ = r; }

	/** Kill child prematurely.
	 *  First, a SIGHUP is sent to the child.
	 *  If that does not end the child process within "tolerance"
	 *  seconds, the SIGKILL signal is sent to the child.
	 *  When the child is dead, the callback is called.
	 */
	void kill(int tolerance = 5);
	///
	string const & command() const { return command_; }

private:
	/// Callback function
	SignalTypePtr signal_;

	/// Commmand line
	string command_;

	/// Process ID of child
	pid_t pid_;

	/// Return value from child
	int retval_;

	///
	pid_t generateChild();

	/// Wait for child process to finish. Updates returncode from child.
	int waitForChild();
};

#endif // FORKEDCALL_H
