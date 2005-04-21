/**
 * \file forkedcontr.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 *
 * A class for the control of child processes launched using
 * fork() and execvp().
 */

#include <config.h>

#include "forkedcontr.h"
#include "forkedcall.h"
#include "lyxfunctional.h"
#include "debug.h"

#ifdef _WIN32
# include <sstream>
# include <windows.h>

#else
# include <cerrno>
# include <csignal>
# include <cstdlib>
# include <unistd.h>
# include <sys/wait.h>

# ifndef CXX_GLOBAL_CSTD
  using std::strerror;
# endif
#endif

#include "frontends/Timeout.h"

#include <boost/bind.hpp>


using std::vector;
using std::endl;
using std::find_if;


#if defined(_WIN32)
string const getChildErrorMessage()
{
	DWORD const error_code = ::GetLastError();

	HLOCAL t_message = 0;
	bool const ok = ::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		0, error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &t_message, 0, 0
		) != 0;

	std::ostringstream ss;
	ss << "LyX: Error waiting for child: " << error_code;

	if (ok) {
		ss << ": " << (LPTSTR)t_message;
		::LocalFree(t_message);
	} else
		ss << ": Error unknown.";

	return STRCONV(ss.str());
}
#endif


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
}


// Timer-call
// Check the list and, if there is a stopped child, emit the signal.
void ForkedcallsController::timer()
{
	ListType::iterator it  = forkedCalls.begin();
	ListType::iterator end = forkedCalls.end();
	while (it != end) {
		ForkedProcess * actCall = *it;
		bool remove_it = false;

#if defined(_WIN32)
		HANDLE const hProcess = HANDLE(actCall->pid());

		DWORD const wait_status = ::WaitForSingleObject(hProcess, 0);

		switch (wait_status) {
		case WAIT_TIMEOUT:
			// Still running
			break;
		case WAIT_OBJECT_0: {
			DWORD exit_code = 0;
			if (!GetExitCodeProcess(hProcess, &exit_code)) {
				lyxerr << "GetExitCodeProcess failed waiting for child\n"
				       << getChildErrorMessage() << std::endl;
				// Child died, so pretend it returned 1
				actCall->setRetValue(1);
			} else {
				actCall->setRetValue(exit_code);
			}
			remove_it = true;
			break;
		}
		case WAIT_FAILED:
			lyxerr << "WaitForSingleObject failed waiting for child\n"
			       << getChildErrorMessage() << std::endl;
			actCall->setRetValue(1);
			remove_it = true;
			break;
		}
#else
		pid_t pid = actCall->pid();
		int stat_loc;
		pid_t const waitrpid = waitpid(pid, &stat_loc, WNOHANG);

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
#endif

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
