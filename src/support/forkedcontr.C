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

#include "forkedcontr.h"
#include "forkedcall.h"
#include "lyxfunctional.h"
#include "debug.h"

#include "frontends/Timeout.h"

#include <boost/bind.hpp>

#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

using std::endl;
using std::find_if;

using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::strerror;
#endif


namespace lyx {
namespace support {

// Ensure, that only one controller exists inside process
ForkedcallsController & ForkedcallsController::get()
{
	static ForkedcallsController singleton;
	return singleton;
}


ForkedcallsController::ForkedcallsController()
{
	timeout_ = new Timeout(100, Timeout::ONETIME);

	timeout_->timeout
		.connect(boost::bind(&ForkedcallsController::timer, this));
}


// open question: should we stop childs here?
// Asger says no: I like to have my xdvi open after closing LyX. Maybe
// I want to print or something.
ForkedcallsController::~ForkedcallsController()
{
	for (ListType::iterator it = forkedCalls.begin();
	     it != forkedCalls.end(); ++it) {
		delete *it;
	}

	delete timeout_;
}


void ForkedcallsController::addCall(ForkedProcess const & newcall)
{
	if (!timeout_->running())
		timeout_->start();

	forkedCalls.push_back(newcall.clone());
	childrenChanged();
}


// Timer-call
// Check the list and, if there is a stopped child, emit the signal.
void ForkedcallsController::timer()
{
	ListType::size_type start_size = forkedCalls.size();

	ListType::iterator it  = forkedCalls.begin();
	ListType::iterator end = forkedCalls.end();
	while (it != end) {
		ForkedProcess * actCall = *it;

		pid_t pid = actCall->pid();
		int stat_loc;
		pid_t const waitrpid = waitpid(pid, &stat_loc, WNOHANG);
		bool remove_it = false;

		if (waitrpid == -1) {
			lyxerr << "LyX: Error waiting for child: "
			       << strerror(errno) << endl;

			// Child died, so pretend it returned 1
			actCall->setRetValue(1);
			remove_it = true;

		} else if (waitrpid == 0) {
			// Still running. Move on to the next child.

		} else if (WIFEXITED(stat_loc)) {
			// Ok, the return value goes into retval.
			actCall->setRetValue(WEXITSTATUS(stat_loc));
			remove_it = true;

		} else if (WIFSIGNALED(stat_loc)) {
			// Child died, so pretend it returned 1
			actCall->setRetValue(1);
			remove_it = true;

		} else if (WIFSTOPPED(stat_loc)) {
			lyxerr << "LyX: Child (pid: " << pid
			       << ") stopped on signal "
			       << WSTOPSIG(stat_loc)
			       << ". Waiting for child to finish." << endl;

		} else {
			lyxerr << "LyX: Something rotten happened while "
				"waiting for child " << pid << endl;

			// Child died, so pretend it returned 1
			actCall->setRetValue(1);
			remove_it = true;
		}

		if (remove_it) {
			forkedCalls.erase(it);

			actCall->emitSignal();
			delete actCall;

			/* start all over: emiting the signal can result
			 * in changing the list (Ab)
			 */
			it = forkedCalls.begin();
		} else {
			++it;
		}
	}

	if (!forkedCalls.empty() && !timeout_->running()) {
		timeout_->start();
	}

	if (start_size != forkedCalls.size())
		childrenChanged();
}


// Return a vector of the pids of all the controlled processes.
vector<pid_t> const ForkedcallsController::getPIDs() const
{
	vector<pid_t> pids;

	if (forkedCalls.empty())
		return pids;

	pids.resize(forkedCalls.size());

	vector<pid_t>::iterator vit = pids.begin();
	for (ListType::const_iterator lit = forkedCalls.begin();
	     lit != forkedCalls.end(); ++lit, ++vit) {
		*vit = (*lit)->pid();
	}

	std::sort(pids.begin(), pids.end());
	return pids;
}


// Get the command string of the process.
string const ForkedcallsController::getCommand(pid_t pid) const
{
	ListType::const_iterator it =
		find_if(forkedCalls.begin(), forkedCalls.end(),
			lyx::compare_memfun(&Forkedcall::pid, pid));

	if (it == forkedCalls.end())
		return string();

	return (*it)->command();
}


// Kill the process prematurely and remove it from the list
// within tolerance secs
void ForkedcallsController::kill(pid_t pid, int tolerance)
{
	ListType::iterator it =
		find_if(forkedCalls.begin(), forkedCalls.end(),
			lyx::compare_memfun(&Forkedcall::pid, pid));

	if (it == forkedCalls.end())
		return;

	(*it)->kill(tolerance);
	forkedCalls.erase(it);

	if (forkedCalls.empty()) {
		timeout_->stop();
	}
}

} // namespace support
} // namespace lyx
