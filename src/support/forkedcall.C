/**
 * \file forkedcall.C
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

#include "forkedcall.h"
#include "forkedcontr.h"
#include "lstrings.h"
#include "lyxlib.h"
#include "filetools.h"
#include "os.h"
#include "debug.h"
#include "frontends/Timeout.h"

#include <boost/bind.hpp>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>

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
			lyx::support::kill(pid, SIGKILL);
		}
	}

	//
	void kill()
	{
		if (pid_ != 0) {
			lyx::support::kill(pid_, SIGKILL);
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

	// Un-UNIX like, but we don't have much use for
	// knowing if a zombie exists, so just reap it first.
	int waitstatus;
	waitpid(pid(), &waitstatus, WNOHANG);

	// Racy of course, but it will do.
	if (::kill(pid(), 0) && errno == ESRCH)
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

	int const tolerance = std::max(0, tol);
	if (tolerance == 0) {
		// Kill it dead NOW!
		Murder::killItDead(0, pid());

	} else {
		int ret = lyx::support::kill(pid(), SIGHUP);

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
	// in a contiguous block of memory.
	char const * const c_str = line.c_str();
	// Don't forget the terminating `\0' character.
	vector<char> vec(c_str, c_str + line.size() + 1);
	// Turn the string into an array of words, each terminated with '\0'.
	std::replace(vec.begin(), vec.end(), ' ', '\0');

	// Build an array of pointers to each word.
	vector<char>::iterator vit = vec.begin();
	vector<char>::iterator vend = vec.end();
	vector<char *> argv;
	char prev = '\0';
	for (; vit != vend; ++vit) {
		if (*vit != '\0' && prev == '\0')
			argv.push_back(&*vit);
		prev = *vit;
	}
	// Strip quotes. Does so naively, assuming that the word begins
	// and ends in quotes.
	vector<char *>::iterator ait = argv.begin();
	vector<char *>::iterator const aend = argv.end();
	for (; ait != aend; ++ait) {
		char * word = *ait;
		std::size_t const len = strlen(word);
		if (len >= 2) {
			char & first = word[0];
			char & last = word[len-1];

			if (first == last &&
			    (first == '\'' || first == '"')) {
				first = '\0';
				last = '\0';
				*ait += 1;
			}
		}
	}

	ait = argv.begin();
	for (; ait != aend; ++ait)
		std::cout << *ait << std::endl;
	argv.push_back(0);

#ifndef __EMX__
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
	pid_t const cpid = spawnvp(P_SESSION|P_DEFAULT|P_MINIMIZE|P_BACKGROUND,
				   argv[0], &*argv.begin());
#endif

	if (cpid < 0) {
		// Error.
		lyxerr << "Could not fork: " << strerror(errno) << endl;
	}

	return cpid;
}

} // namespace support
} // namespace lyx
