// -*- C++ -*-
/**
 * \file forkedcontr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * A class for the control of child processes launched using
 * fork() and execvp().
 */

#ifndef FORKEDCONTR_H
#define FORKEDCONTR_H

#include <boost/shared_ptr.hpp>
#include <csignal>
//#include <sys/types.h> // needed for pid_t
#include <list>
#include <vector>

namespace lyx {
namespace support {

class ForkedProcess;

class ForkedcallsController {
public:
	/// Get hold of the only controller that can exist inside the process.
	static ForkedcallsController & get();

	/// Are there any completed child processes to be cleaned-up after?
	bool processesCompleted() const { return current_child != -1; }

	/** Those child processes that are found to have finished are removed
	 *  from the list and their callback function is passed the final
	 *  return state.
 	 */
	void handleCompletedProcesses();

	/// Add a new child process to the list of controlled processes.
	void addCall(ForkedProcess const &);

	/** Kill this process prematurely and remove it from the list.
	 *  The process is killed within tolerance secs.
	 *  See forkedcall.[Ch] for details.
	 */
	void kill(pid_t, int tolerance = 5);

	struct Data {
		Data() : pid(0), status(0) {}
		pid_t pid;
		int status;
	};

	/** These data are used by the SIGCHLD handler to populate a list
	 *  of child processes that have completed and been reaped.
	 *  The associated signals are then emitted within the main LyX
	 *  event loop.
	 */
	std::vector<Data> reaped_children;
	sig_atomic_t current_child;

private:
	ForkedcallsController();
	ForkedcallsController(ForkedcallsController const &);
	~ForkedcallsController();

	typedef boost::shared_ptr<ForkedProcess> ForkedProcessPtr;
	typedef std::list<ForkedProcessPtr> ListType;
	typedef ListType::iterator iterator;

	iterator find_pid(pid_t);

	/// The child processes
	ListType forkedCalls;

	/// Used to block SIGCHLD signals.
	sigset_t newMask, oldMask;
};

} // namespace support
} // namespace lyx

#endif // FORKEDCONTR_H
