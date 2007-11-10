/**
 * \file Forkedcall.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
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

#include <config.h>

#include "support/Forkedcall.h"
#include "support/ForkedcallsController.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/filetools.h"
#include "support/os.h"
#include "support/Timeout.h"

#include "debug.h"

#include <boost/bind.hpp>

#include <vector>
#include <cerrno>

#ifdef _WIN32
# define SIGHUP 1
# define SIGKILL 9
# include <process.h>
# include <windows.h>

#else
# include <csignal>
# include <cstdlib>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
# include <sys/wait.h>
#endif

using std::endl;
using std::string;
using std::vector;

#ifndef CXX_GLOBAL_CSTD
using std::strerror;
#endif

namespace lyx {
namespace support {


namespace {

class Murder : public boost::signals::trackable {
public:
	//
	static void killItDead(int secs, pid_t pid)
	{
		if (secs > 0) {
			new Murder(secs, pid);
		} else if (pid != 0) {
			support::kill(pid, SIGKILL);
		}
	}

	//
	void kill()
	{
		if (pid_ != 0) {
			support::kill(pid_, SIGKILL);
		}
		lyxerr << "Killed " << pid_ << std::endl;
		delete this;
	}

private:
	//
	Murder(int secs, pid_t pid)
		: timeout_(0), pid_(pid)
	{
		timeout_ = new Timeout(1000*secs, Timeout::ONETIME);
		timeout_->timeout.connect(boost::bind(&Murder::kill, this));
		timeout_->start();
	}

	//
	~Murder()
	{
		delete timeout_;
	}
	//
	Timeout * timeout_;
	//
	pid_t pid_;
};

} // namespace anon


ForkedProcess::ForkedProcess()
	: pid_(0), retval_(0)
{}


void ForkedProcess::emitSignal()
{
	if (signal_.get()) {
		signal_->operator()(pid_, retval_);
	}
}


// Spawn the child process
int ForkedProcess::run(Starttype type)
{
	retval_  = 0;
	pid_ = generateChild();
	if (pid_ <= 0) { // child or fork failed.
		retval_ = 1;
		return retval_;
	}

	switch (type) {
	case Wait:
		retval_ = waitForChild();
		break;
	case DontWait: {
		// Integrate into the Controller
		ForkedcallsController & contr = ForkedcallsController::get();
		contr.addCall(*this);
		break;
	}
	}

	return retval_;
}


bool ForkedProcess::running() const
{
	if (!pid())
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
	if (pid() == 0) {
		lyxerr << "Can't kill non-existent process!" << endl;
		return;
	}

	// The weird (std::max)(a,b) signature prevents expansion
	// of an evil MSVC macro.
	int const tolerance = (std::max)(0, tol);
	if (tolerance == 0) {
		// Kill it dead NOW!
		Murder::killItDead(0, pid());

	} else {
		int ret = support::kill(pid(), SIGHUP);

		// The process is already dead if wait_for_death is false
		bool const wait_for_death = (ret == 0 && errno != ESRCH);

		if (wait_for_death) {
			Murder::killItDead(tolerance, pid());
		}
	}
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
			       << getChildErrorMessage() << std::endl;
		} else
			retval_ = exit_code;
		break;
	}
	case WAIT_FAILED:
		lyxerr << "WaitForSingleObject failed waiting for child\n"
		       << getChildErrorMessage() << std::endl;
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


int Forkedcall::startscript(Starttype wait, string const & what)
{
	if (wait != Wait) {
		retval_ = startscript(what, SignalTypePtr());
		return retval_;
	}

	command_ = what;
	signal_.reset();
	return run(Wait);
}


int Forkedcall::startscript(string const & what, SignalTypePtr signal)
{
	command_ = what;
	signal_  = signal;

	return run(DontWait);
}


// generate child in background
int Forkedcall::generateChild()
{
	string line = trim(command_);
	if (line.empty())
		return 1;

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
	//    delimit the entire word.
	char inside_quote = 0;
	vector<char>::iterator it = vec.begin();
	vector<char>::iterator const end = vec.end();
	for (; it != end; ++it) {
		char const c = *it;
		if (!inside_quote) {
			if (c == ' ')
				*it = '\0';
			else if (c == '\'' || c == '"') {
#if defined (_WIN32)
				// How perverse!
				// spawnvp *requires* the quotes or it will
				// split the arg at the internal whitespace!
				// Make shure the quote is a DOS-style one.
				*it = '"';
#else
				*it = '\0';
#endif
				inside_quote = c;
			}
		} else if (c == inside_quote) {
#if defined (_WIN32)
			*it = '"';
#else
			*it = '\0';
#endif
			inside_quote = 0;
		}
	}

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
		       << "\n\tInterpretted as:\n\n";
		for (; ait != aend; ++ait)
			if (*ait)
				lyxerr << '\t'<< *ait << '\n';
		lyxerr << "</command>" << std::endl;
	}

#ifdef _WIN32
	pid_t const cpid = spawnvp(_P_NOWAIT, argv[0], &*argv.begin());
#else // POSIX
	pid_t const cpid = ::fork();
	if (cpid == 0) {
		// Child
		execvp(argv[0], &*argv.begin());

		// If something goes wrong, we end up here
		lyxerr << "execvp of \"" << command_ << "\" failed: "
		       << strerror(errno) << endl;
		_exit(1);
	}
#endif

	if (cpid < 0) {
		// Error.
		lyxerr << "Could not fork: " << strerror(errno) << endl;
	}

	return cpid;
}

} // namespace support
} // namespace lyx
