/**
 * \file forkedcontr.C
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

#include <config.h>

#include "support/forkedcontr.h"
#include "support/forkedcall.h"

#include "debug.h"

#include <boost/bind.hpp>

#include <cerrno>
#include <cstdlib>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/wait.h>

using boost::bind;

using std::endl;
using std::equal_to;
using std::find_if;

using std::string;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::signal;
using std::strerror;
#endif


namespace lyx {
namespace support {

/* The forkedcall controller code handles finished child processes in a
   two-stage process.

   1. It uses the SIGCHLD signal emitted by the system when the child process
      finishes to reap the resulting zombie. The handler routine also
      updates an internal list of completed children.
   2. The signals associated with these completed children are then emitted
      as part of the main LyX event loop.

   The guiding philosophy is that zombies are a global resource that should
   be reaped as soon as possible whereas an internal list of dead children
   is not. Indeed, to emit the signals within the asynchronous handler
   routine would result in unsafe code.

   The signal handler is guaranteed to be safe even though it may not be
   atomic:

   int completed_child_status;
   sig_atomic_t completed_child_pid;

   extern "C"
   void child_handler(int)
   {
     // Clean up the child process.
     completed_child_pid = wait(&completed_child_status);
   }

   (See the signals tutorial at http://tinyurl.com/3h82w.)

   It's safe because:
   1. wait(2) is guaranteed to be async-safe.
   2. child_handler handles only SIGCHLD signals so all subsequent
      SIGCHLD signals are blocked from entering the handler until the
      existing signal is processed.

   This handler performs 'half' of the necessary clean up after a
   completed child process. It prevents us leaving a stream of zombies
   behind but does not go on to tell the main LyX program to finish the
   clean-up by emitting the stored signal. That would most definitely
   not be safe.

   The only problem with the above is that the global stores
   completed_child_status, completed_child_pid may be overwritten before
   the clean-up is completed in the main loop.

   However, the code in child_handler can be extended to fill an array of
   completed processes. Everything remains safe so long as no 'unsafe'
   functions are called. (See the list of async-safe functions at
   http://tinyurl.com/3h82w.)

   struct child_data {
     pid_t pid;
     int status;
   };

   // This variable may need to be resized in the main program
   // as and when a new process is forked. This resizing must be
   // protected with sigprocmask
   std::vector<child_data> reaped_children;
   sig_atomic_t current_child = -1;

   extern "C"
   void child_handler(int)
   {
     child_data & store = reaped_children[++current_child];
     // Clean up the child process.
     store.pid = wait(&store.status);
   }

   That is, we build up a list of completed children in anticipation of
   the main loop then looping over this list and invoking any associated
   callbacks etc. The nice thing is that the main loop needs only to
   check the value of 'current_child':

   if (current_child != -1)
     handleCompletedProcesses();

   handleCompletedProcesses now loops over only those child processes
   that have completed (ie, those stored in reaped_children). It blocks
   any subsequent SIGCHLD signal whilst it does so:

   // Used to block SIGCHLD signals.
   sigset_t newMask, oldMask;

   ForkedcallsController::ForkedcallsController()
   {
     reaped_children.resize(50);
     signal(SIGCHLD, child_handler);

     sigemptyset(&oldMask);
     sigemptyset(&newMask);
     sigaddset(&newMask, SIGCHLD);
   }

   void ForkedcallsController::handleCompletedProcesses()
   {
     if (current_child == -1)
       return;

     // Block the SIGCHLD signal.
     sigprocmask(SIG_BLOCK, &newMask, &oldMask);

     for (int i = 0; i != 1+current_child; ++i) {
       child_data & store = reaped_children[i];
       // Go on to handle the child process
       ...
     }

     // Unblock the SIGCHLD signal and restore the old mask.
     sigprocmask(SIG_SETMASK, &oldMask, 0);
   }

   Voilà! An efficient, elegant and *safe* mechanism to handle child processes.
*/

namespace {

extern "C"
void child_handler(int)
{
	ForkedcallsController & fcc = ForkedcallsController::get();

	// Be safe
	typedef vector<ForkedcallsController::Data>::size_type size_type;
	if (size_type(fcc.current_child + 1) >= fcc.reaped_children.size())
		return;

	ForkedcallsController::Data & store =
		fcc.reaped_children[++fcc.current_child];
	// Clean up the child process.
	store.pid = wait(&store.status);
}

} // namespace anon


// Ensure, that only one controller exists inside process
ForkedcallsController & ForkedcallsController::get()
{
	static ForkedcallsController singleton;
	return singleton;
}


ForkedcallsController::ForkedcallsController()
	: reaped_children(50), current_child(-1)
{
	signal(SIGCHLD, child_handler);

	sigemptyset(&oldMask);
	sigemptyset(&newMask);
	sigaddset(&newMask, SIGCHLD);
}


// open question: should we stop childs here?
// Asger says no: I like to have my xdvi open after closing LyX. Maybe
// I want to print or something.
ForkedcallsController::~ForkedcallsController()
{
	signal(SIGCHLD, SIG_DFL);
}


void ForkedcallsController::addCall(ForkedProcess const & newcall)
{
	forkedCalls.push_back(newcall.clone());

	if (forkedCalls.size() > reaped_children.size()) {
		// Block the SIGCHLD signal.
		sigprocmask(SIG_BLOCK, &newMask, &oldMask);

		reaped_children.resize(2*reaped_children.size());

		// Unblock the SIGCHLD signal and restore the old mask.
		sigprocmask(SIG_SETMASK, &oldMask, 0);
	}
}


ForkedcallsController::iterator
 ForkedcallsController::find_pid(pid_t pid)
{
	return find_if(forkedCalls.begin(), forkedCalls.end(),
		       bind(equal_to<pid_t>(),
			    bind(&Forkedcall::pid, _1),
			    pid));
}


// Kill the process prematurely and remove it from the list
// within tolerance secs
void ForkedcallsController::kill(pid_t pid, int tolerance)
{
	ListType::iterator it = find_pid(pid);
	if (it == forkedCalls.end())
		return;

	(*it)->kill(tolerance);
	forkedCalls.erase(it);
}


// Check the list of dead children and emit any associated signals.
void ForkedcallsController::handleCompletedProcesses()
{
	if (current_child == -1)
		return;

	// Block the SIGCHLD signal.
	sigprocmask(SIG_BLOCK, &newMask, &oldMask);

	for (int i = 0; i != 1 + current_child; ++i) {
		Data & store = reaped_children[i];

		if (store.pid == -1) {
			// Might happen perfectly innocently, eg as a result
			// of the system (3) call.
			if (errno)
				lyxerr << "LyX: Error waiting for child: "
				       << strerror(errno) << endl;
			continue;
		}

		ListType::iterator it = find_pid(store.pid);
		if (it == forkedCalls.end())
			// Eg, child was run in blocking mode
			continue;

		ListType::value_type child = (*it);
		bool remove_it = false;

		if (WIFEXITED(store.status)) {
			// Ok, the return value goes into retval.
			child->setRetValue(WEXITSTATUS(store.status));
			remove_it = true;

		} else if (WIFSIGNALED(store.status)) {
			// Child died, so pretend it returned 1
			child->setRetValue(1);
			remove_it = true;

		} else if (WIFSTOPPED(store.status)) {
			lyxerr << "LyX: Child (pid: " << store.pid
			       << ") stopped on signal "
			       << WSTOPSIG(store.status)
			       << ". Waiting for child to finish." << endl;

		} else {
			lyxerr << "LyX: Something rotten happened while "
			       << "waiting for child " << store.pid << endl;

			// Child died, so pretend it returned 1
			child->setRetValue(1);
			remove_it = true;
		}

		if (remove_it) {
			child->emitSignal();
			forkedCalls.erase(it);
		}
	}

	// Reset the counter
	current_child = -1;

	// Unblock the SIGCHLD signal and restore the old mask.
	sigprocmask(SIG_SETMASK, &oldMask, 0);
}

} // namespace support
} // namespace lyx
