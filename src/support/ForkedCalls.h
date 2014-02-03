// -*- C++ -*-
/**
 * \file ForkedCalls.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Angus Leeming
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORKEDCALLS_H
#define FORKEDCALLS_H

#include "support/shared_ptr.h"
#include "support/strfwd.h"
#include <boost/signal.hpp>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

namespace lyx {
namespace support {

class ForkedProcess {
public:
	///
	enum Starttype {
		///
		Wait,
		///
		DontWait
	};

	///
	ForkedProcess();
	///
	virtual ~ForkedProcess() {}
	///
	virtual shared_ptr<ForkedProcess> clone() const = 0;

	/** A SignalType signal can be emitted once the forked process
	 *  has finished. It passes:
	 *  the PID of the child and;
	 *  the return value from the child.
	 *
	 *  We use a signal rather than simply a callback function so that
	 *  we can return easily to C++ methods, rather than just globally
	 *  accessible functions.
	 */
	typedef boost::signal<void(pid_t, int)> SignalType;

	/** The signal is connected in the calling routine to the desired
	 *  slot. We pass a shared_ptr rather than a reference to the signal
	 *  because it is eminently possible for the instance of the calling
	 *  class (and hence the signal) to be destructed before the forked
	 *  call is complete.
	 *
	 *  It doesn't matter if the slot disappears, SigC takes care of that.
	 */
	typedef shared_ptr<SignalType> SignalTypePtr;

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

	/// Returns the identifying command (for display in the GUI perhaps).
	std::string const & command() const { return command_; }

	/// is the process running ?
	bool running() const;

	/** Kill child prematurely.
	 *  First, a SIGHUP is sent to the child.
	 *  If that does not end the child process within "tolerance"
	 *  seconds, the SIGKILL signal is sent to the child.
	 *  When the child is dead, the callback is called.
	 */
	void kill(int tolerance = 5);

	/// Returns true if this is a child process
	static bool iAmAChild() { return IAmAChild; }

protected:
	/** Spawn the child process.
	 *  Returns returncode from child.
	 */
	int run(Starttype type);

	/// implement our own version of fork()
	/// it just returns -1 if ::fork() is not defined
	/// otherwise, it forks and sets the global child-process
	/// boolean IAmAChild
	pid_t fork();

	/// Callback function
	SignalTypePtr signal_;

	/// identifying command (for display in the GUI perhaps).
	std::string command_;

	/// Process ID of child
	pid_t pid_;

	/// Return value from child
	int retval_;
private:
	/// generate child in background
	virtual int generateChild() = 0;

	///
	static bool IAmAChild;

	/// Wait for child process to finish. Updates returncode from child.
	int waitForChild();
};


/** 
 * An instance of class ForkedCall represents a single child process.
 *
 * Class ForkedCall uses fork() and execvp() to lauch the child process.
 *
 * Once launched, control is returned immediately to the parent process
 * but a Signal can be emitted upon completion of the child.
 *
 * The child process is not killed when the ForkedCall instance goes out of
 * scope, but it can be killed by an explicit invocation of the kill() member
 * function.
 */

class ForkedCall : public ForkedProcess {
public:
	///
	ForkedCall(std::string const & path = empty_string());
	///
	virtual shared_ptr<ForkedProcess> clone() const {
		return shared_ptr<ForkedProcess>(new ForkedCall(*this));
	}

	/** Start the child process.
	 *
	 *  The command "what" is passed to execvp() for execution. "$$s" is
	 *  replaced accordingly by commandPrep().
	 *
	 *  There are two startScript commands available. They differ in that
	 *  the second receives a signal that is executed on completion of
	 *  the command. This makes sense only for a command executed
	 *  in the background, ie DontWait.
	 *
	 *  The other startscript command can be executed either blocking
	 *  or non-blocking, but no signal will be emitted on finishing.
	 */
	int startScript(Starttype, std::string const & what);

	///
	int startScript(std::string const & what, SignalTypePtr);

private:
	///
	virtual int generateChild();
	///
	std::string cmd_prefix_;
};


/**
 * This interfaces a queue of forked processes. In order not to
 * hose the system with multiple processes running simultaneously, you can
 * request the addition of your process to this queue and it will be
 * executed when its turn comes.
 *
 */

namespace ForkedCallQueue {

ForkedCall::SignalTypePtr add(std::string const & process);
/// Query whether the queue is running a forked process now.
bool running();

}


/**
 * Control of child processes launched using fork() and execvp().
 */

namespace ForkedCallsController {

/// Add a new child process to the list of controlled processes.
void addCall(ForkedProcess const &);

/** Those child processes that are found to have finished are removed
 *  from the list and their callback function is passed the final
 *  return state.
 */
void handleCompletedProcesses();

/** Kill this process prematurely and remove it from the list.
 *  The process is killed within tolerance secs.
 *  See forkedcall.[Ch] for details.
 */
void kill(pid_t, int tolerance = 5);

} // namespace ForkedCallsController


#if defined(_WIN32)
// a wrapper for GetLastError() and FormatMessage().
std::string const getChildErrorMessage();
#endif

} // namespace support
} // namespace lyx

#endif // FORKEDCALLS_H
