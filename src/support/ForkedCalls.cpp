/**
 * \file ForkedCalls.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Angus Leeming
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/ForkedCalls.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/Timeout.h"

#include "support/bind.h"

#include <cerrno>
#include <queue>
#include <sstream>
#include <utility>
#include <vector>

#ifdef _WIN32
# define SIGHUP 1
# define SIGKILL 9
# include <windows.h>
# include <process.h>
# undef max
#else
# include <csignal>
# include <cstdlib>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
# include <sys/wait.h>
#endif

using namespace std;



namespace lyx {
namespace support {

namespace {

/////////////////////////////////////////////////////////////////////
//
// Murder
//
/////////////////////////////////////////////////////////////////////

class Murder : public boost::signals::trackable {
public:
	//
	static void killItDead(int secs, pid_t pid)
	{
		if (secs > 0)
			new Murder(secs, pid);
		else if (pid != 0)
			support::kill(pid, SIGKILL);
	}

	//
	void kill()
	{
		if (pid_ != 0)
			support::kill(pid_, SIGKILL);
		lyxerr << "Killed " << pid_ << endl;
		delete this;
	}

private:
	//
	Murder(int secs, pid_t pid)
		: timeout_(1000*secs, Timeout::ONETIME), pid_(pid)
	{
		timeout_.timeout.connect(lyx::bind(&Murder::kill, this));
		timeout_.start();
	}

	//
	Timeout timeout_;
	//
	pid_t pid_;
};

} // namespace anon


/////////////////////////////////////////////////////////////////////
//
// ForkedProcess
//
/////////////////////////////////////////////////////////////////////

ForkedProcess::ForkedProcess()
	: pid_(0), retval_(0)
{}


bool ForkedProcess::IAmAChild = false;


void ForkedProcess::emitSignal()
{
	if (signal_) {
		signal_->operator()(pid_, retval_);
	}
}


// Spawn the child process
int ForkedProcess::run(Starttype type)
{
	retval_ = 0;
	pid_ = generateChild();
	if (pid_ <= 0) { // child or fork failed.
		retval_ = 1;
		if (pid_ == 0)
			//we also do this in fork(), too, but maybe someone will try
			//to bypass that
			IAmAChild = true; 
		return retval_;
	}

	switch (type) {
	case Wait:
		retval_ = waitForChild();
		break;
	case DontWait: {
		// Integrate into the Controller
		ForkedCallsController::addCall(*this);
		break;
	}
	}

	return retval_;
}


bool ForkedProcess::running() const
{
	if (pid() <= 0)
		return false;

#if !defined (_WIN32)
	// Un-UNIX like, but we don't have much use for
	// knowing if a zombie exists, so just reap it first.
	int waitstatus;
	waitpid(pid(), &waitstatus, WNOHANG);
#endif

	// Racy of course, but it will do.
	if (support::kill(pid(), 0) && errno == ESRCH)
		return false;
	return true;
}


void ForkedProcess::kill(int tol)
{
	lyxerr << "ForkedProcess::kill(" << tol << ')' << endl;
	if (pid() <= 0) {
		lyxerr << "Can't kill non-existent process!" << endl;
		return;
	}

	int const tolerance = max(0, tol);
	if (tolerance == 0) {
		// Kill it dead NOW!
		Murder::killItDead(0, pid());
	} else {
		int ret = support::kill(pid(), SIGHUP);

		// The process is already dead if wait_for_death is false
		bool const wait_for_death = (ret == 0 && errno != ESRCH);

		if (wait_for_death)
			Murder::killItDead(tolerance, pid());
	}
}


pid_t ForkedProcess::fork() {
#if !defined (HAVE_FORK)
	return -1;
#else
	pid_t pid = ::fork();
	if (pid == 0)
		IAmAChild = true;
	return pid;
#endif
}


// Wait for child process to finish. Returns returncode from child.
int ForkedProcess::waitForChild()
{
	// We'll pretend that the child returns 1 on all error conditions.
	retval_ = 1;

#if defined (_WIN32)
	HANDLE const hProcess = HANDLE(pid_);

	DWORD const wait_status = ::WaitForSingleObject(hProcess, INFINITE);

	switch (wait_status) {
	case WAIT_OBJECT_0: {
		DWORD exit_code = 0;
		if (!GetExitCodeProcess(hProcess, &exit_code)) {
			lyxerr << "GetExitCodeProcess failed waiting for child\n"
			       << getChildErrorMessage() << endl;
		} else
			retval_ = exit_code;
		break;
	}
	case WAIT_FAILED:
		lyxerr << "WaitForSingleObject failed waiting for child\n"
		       << getChildErrorMessage() << endl;
		break;
	}

#else
	int status;
	bool wait = true;
	while (wait) {
		pid_t waitrpid = waitpid(pid_, &status, WUNTRACED);
		if (waitrpid == -1) {
			lyxerr << "LyX: Error waiting for child:"
			       << strerror(errno) << endl;
			wait = false;
		} else if (WIFEXITED(status)) {
			// Child exited normally. Update return value.
			retval_ = WEXITSTATUS(status);
			wait = false;
		} else if (WIFSIGNALED(status)) {
			lyxerr << "LyX: Child didn't catch signal "
			       << WTERMSIG(status)
			       << "and died. Too bad." << endl;
			wait = false;
		} else if (WIFSTOPPED(status)) {
			lyxerr << "LyX: Child (pid: " << pid_
			       << ") stopped on signal "
			       << WSTOPSIG(status)
			       << ". Waiting for child to finish." << endl;
		} else {
			lyxerr << "LyX: Something rotten happened while "
				"waiting for child " << pid_ << endl;
			wait = false;
		}
	}
#endif
	return retval_;
}


/////////////////////////////////////////////////////////////////////
//
// ForkedCall
//
/////////////////////////////////////////////////////////////////////

ForkedCall::ForkedCall(string const & path)
	: cmd_prefix_(to_filesystem8bit(from_utf8(latexEnvCmdPrefix(path))))
{}


int ForkedCall::startScript(Starttype wait, string const & what)
{
	if (wait != Wait) {
		retval_ = startScript(what, SignalTypePtr());
		return retval_;
	}

	command_ = commandPrep(trim(what));
	signal_.reset();
	return run(Wait);
}


int ForkedCall::startScript(string const & what, SignalTypePtr signal)
{
	command_ = commandPrep(trim(what));
	signal_  = signal;

	return run(DontWait);
}


// generate child in background
int ForkedCall::generateChild()
{
	if (command_.empty())
		return 1;

	// Make sure that a V2 python is run, if available.
	string const line = cmd_prefix_ +
		(prefixIs(command_, "python -tt")
		 ? os::python() + command_.substr(10) : command_);

#if !defined (_WIN32)
	// POSIX

	// Split the input command up into an array of words stored
	// in a contiguous block of memory. The array contains pointers
	// to each word.
	// Don't forget the terminating `\0' character.
	char const * const c_str = line.c_str();
	vector<char> vec(c_str, c_str + line.size() + 1);

	// Splitting the command up into an array of words means replacing
	// the whitespace between words with '\0'. Life is complicated
	// however, because words protected by quotes can contain whitespace.
	//
	// The strategy we adopt is:
	// 1. If we're not inside quotes, then replace white space with '\0'.
	// 2. If we are inside quotes, then don't replace the white space
	//    but do remove the quotes themselves. We do this naively by
	//    replacing the quote with '\0' which is fine if quotes
	//    delimit the entire word. However, if quotes do not delimit the
	//    entire word (i.e., open quote is inside word), simply discard
	//    them such as not to break the current word.
	char inside_quote = 0;
	char c_before_open_quote = ' ';
	vector<char>::iterator it = vec.begin();
	vector<char>::iterator itc = vec.begin();
	vector<char>::iterator const end = vec.end();
	for (; it != end; ++it, ++itc) {
		char const c = *it;
		if (!inside_quote) {
			if (c == '\'' || c == '"') {
				if (c_before_open_quote == ' ')
					*itc = '\0';
				else
					--itc;
				inside_quote = c;
			} else {
				if (c == ' ')
					*itc = '\0';
				else
					*itc = c;
				c_before_open_quote = c;
			}
		} else if (c == inside_quote) {
			if (c_before_open_quote == ' ')
				*itc = '\0';
			else
				--itc;
			inside_quote = 0;
		} else
			*itc = c;
	}

	// Clear what remains.
	for (; itc != end; ++itc)
		*itc = '\0';

	// Build an array of pointers to each word.
	it = vec.begin();
	vector<char *> argv;
	char prev = '\0';
	for (; it != end; ++it) {
		if (*it != '\0' && prev == '\0')
			argv.push_back(&*it);
		prev = *it;
	}
	argv.push_back(0);

	// Debug output.
	if (lyxerr.debugging(Debug::FILES)) {
		vector<char *>::iterator ait = argv.begin();
		vector<char *>::iterator const aend = argv.end();
		lyxerr << "<command>\n\t" << line
		       << "\n\tInterpreted as:\n\n";
		for (; ait != aend; ++ait)
			if (*ait)
				lyxerr << '\t'<< *ait << '\n';
		lyxerr << "</command>" << endl;
	}

	pid_t const cpid = ::fork();
	if (cpid == 0) {
		// Child
		execvp(argv[0], &*argv.begin());

		// If something goes wrong, we end up here
		lyxerr << "execvp of \"" << command_ << "\" failed: "
		       << strerror(errno) << endl;
		_exit(1);
	}
#else
	// Windows

	pid_t cpid = -1;

	STARTUPINFO startup; 
	PROCESS_INFORMATION process; 

	memset(&startup, 0, sizeof(STARTUPINFO));
	memset(&process, 0, sizeof(PROCESS_INFORMATION));
    
	startup.cb = sizeof(STARTUPINFO);

	if (CreateProcess(0, (LPSTR)line.c_str(), 0, 0, FALSE,
		CREATE_NO_WINDOW, 0, 0, &startup, &process)) {
		CloseHandle(process.hThread);
		cpid = (pid_t)process.hProcess;
	}
#endif

	if (cpid < 0) {
		// Error.
		lyxerr << "Could not fork: " << strerror(errno) << endl;
	}

	return cpid;
}


/////////////////////////////////////////////////////////////////////
//
// ForkedCallQueue
//
/////////////////////////////////////////////////////////////////////

namespace ForkedCallQueue {

/// A process in the queue
typedef pair<string, ForkedCall::SignalTypePtr> Process;
/** Add a process to the queue. Processes are forked sequentially
 *  only one is running at a time.
 *  Connect to the returned signal and you'll be informed when
 *  the process has ended.
 */
ForkedCall::SignalTypePtr add(string const & process);

/// in-progress queue
static queue<Process> callQueue_;

/// flag whether queue is running
static bool running_ = 0;

///
void startCaller();
///
void stopCaller();
///
void callback(pid_t, int);

ForkedCall::SignalTypePtr add(string const & process)
{
	ForkedCall::SignalTypePtr ptr;
	ptr.reset(new ForkedCall::SignalType);
	callQueue_.push(Process(process, ptr));
	if (!running_)
		startCaller();
	return ptr;
}


void callNext()
{
	if (callQueue_.empty())
		return;
	Process pro = callQueue_.front();
	callQueue_.pop();
	// Bind our chain caller
	pro.second->connect(lyx::bind(&ForkedCallQueue::callback, _1, _2));
	ForkedCall call;
	//If we fail to fork the process, then emit the signal
	//to tell the outside world that it failed.
	if (call.startScript(pro.first, pro.second) > 0)
		pro.second->operator()(0,1);
}


void callback(pid_t, int)
{
	if (callQueue_.empty())
		stopCaller();
	else
		callNext();
}


void startCaller()
{
	LYXERR(Debug::GRAPHICS, "ForkedCallQueue: waking up");
	running_ = true ;
	callNext();
}


void stopCaller()
{
	running_ = false ;
	LYXERR(Debug::GRAPHICS, "ForkedCallQueue: I'm going to sleep");
}


bool running()
{
	return running_;
}

} // namespace ForkedCallsQueue



/////////////////////////////////////////////////////////////////////
//
// ForkedCallsController
//
/////////////////////////////////////////////////////////////////////

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

	ostringstream ss;
	ss << "LyX: Error waiting for child: " << error_code;

	if (ok) {
		ss << ": " << (LPTSTR)t_message;
		::LocalFree(t_message);
	} else
		ss << ": Error unknown.";

	return ss.str();
}
#endif


namespace ForkedCallsController {

typedef shared_ptr<ForkedProcess> ForkedProcessPtr;
typedef list<ForkedProcessPtr> ListType;
typedef ListType::iterator iterator;


/// The child processes
static ListType forkedCalls;

iterator find_pid(pid_t pid)
{
	return find_if(forkedCalls.begin(), forkedCalls.end(),
			    lyx::bind(equal_to<pid_t>(),
			    lyx::bind(&ForkedCall::pid, _1),
			    pid));
}


void addCall(ForkedProcess const & newcall)
{
	forkedCalls.push_back(newcall.clone());
}


// Check the list of dead children and emit any associated signals.
void handleCompletedProcesses()
{
	ListType::iterator it  = forkedCalls.begin();
	ListType::iterator end = forkedCalls.end();
	while (it != end) {
		ForkedProcessPtr actCall = *it;
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
				       << getChildErrorMessage() << endl;
				// Child died, so pretend it returned 1
				actCall->setRetValue(1);
			} else {
				actCall->setRetValue(exit_code);
			}
			CloseHandle(hProcess);
			remove_it = true;
			break;
		}
		case WAIT_FAILED:
			lyxerr << "WaitForSingleObject failed waiting for child\n"
			       << getChildErrorMessage() << endl;
			actCall->setRetValue(1);
			CloseHandle(hProcess);
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

			/* start all over: emiting the signal can result
			 * in changing the list (Ab)
			 */
			it = forkedCalls.begin();
		} else {
			++it;
		}
	}
}


// Kill the process prematurely and remove it from the list
// within tolerance secs
void kill(pid_t pid, int tolerance)
{
	ListType::iterator it = find_pid(pid);
	if (it == forkedCalls.end())
		return;

	(*it)->kill(tolerance);
	forkedCalls.erase(it);
}

} // namespace ForkedCallsController

} // namespace support
} // namespace lyx
