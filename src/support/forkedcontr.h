// -*- C++ -*-
/**
 * \file forkedcontr.h
 * Copyright 2001 The LyX Team
 * Read COPYING
 *
 * \author Asger Alstrup Nielsen
 * \author Angus Leeming
 *
 * A class for the control of child processes launched using
 * fork() and execvp().
 */

#ifndef FORKEDCONTR_H
#define FORKEDCONTR_H

#include <list>
#include <vector>
#include "LString.h"
#include <sigc++/signal_system.h>

#ifdef __GNUG__
#pragma interface
#endif

class Forkedcall;
class Timeout;

class ForkedcallsController : public SigC::Object {
public:
	/// Get hold of the only controller that can exist inside the process.
        static ForkedcallsController & get();

	/// Add a new child process to the list of controlled processes.
	void addCall(Forkedcall const & newcall);

	/** This method is connected to the timer. Every XX ms it is called
	 *  so that we can check on the status of the children. Those that
	 *  are found to have finished are removed from the list and their
	 *  callback function is passed the final return state.
	 */
	void timer();

	/// Return a vector of the pids of all the controlled processes.
	std::vector<pid_t> const getPIDs() const;

	/// Get the command string of the process.
	string const getCommand(pid_t) const;

	/** Kill this process prematurely and remove it from the list.
	 *  The process is killed within tolerance secs.
	 *  See forkedcall.[Ch] for details.
	 */
	void kill(pid_t, int tolerance = 5);

	/// Signal emitted when the list of current child processes changes.
	SigC::Signal0<void> childrenChanged;
	
private:
	/// Can't create multiple instances of ForkedcallsController.
	ForkedcallsController();
	///
	ForkedcallsController(ForkedcallsController const &);
	///
	~ForkedcallsController();

	/// The child processes
	typedef std::list<Forkedcall *> ListType;
	///
	ListType forkedCalls;

	/** The timer. Enables us to check the status of the children
	 *  every XX ms and to invoke a callback on completion.
	 */
        Timeout * timeout_;
};

#endif // FORKEDCONTR_H
