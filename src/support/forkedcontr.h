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
#include <sys/types.h> // needed for pid_t
#include <list>

class Timeout;

namespace lyx {
namespace support {

class ForkedProcess;

class ForkedcallsController {
public:
	/// Get hold of the only controller that can exist inside the process.
	static ForkedcallsController & get();

	/// Add a new child process to the list of controlled processes.
	void addCall(ForkedProcess const &);

	/** Kill this process prematurely and remove it from the list.
	 *  The process is killed within tolerance secs.
	 *  See forkedcall.[Ch] for details.
	 */
	void kill(pid_t, int tolerance = 5);

private:
	ForkedcallsController();
	ForkedcallsController(ForkedcallsController const &);
	~ForkedcallsController();

	/** This method is connected to the timer. Every XX ms it is called
	 *  so that we can check on the status of the children. Those that
	 *  are found to have finished are removed from the list and their
	 *  callback function is passed the final return state.
	 */
	void timer();

	/// The child processes
	typedef boost::shared_ptr<ForkedProcess> ForkedProcessPtr;
	typedef std::list<ForkedProcessPtr> ListType;
	///
	ListType forkedCalls;

	/** The timer. Enables us to check the status of the children
	 *  every XX ms and to invoke a callback on completion.
	 */
	Timeout * timeout_;
};

} // namespace support
} // namespace lyx

#endif // FORKEDCONTR_H
