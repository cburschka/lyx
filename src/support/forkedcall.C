/**
 *  \file forkedcall.C
 *  Copyright 2002 the LyX Team
 *  Read the file COPYING
 *
 * \author Asger Alstrup
 *
 * Interface cleaned up by
 * \author Angus Leeming <a.leeming@ic.ac.uk>
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

#ifdef __GNUG__
#pragma implementation
#endif

#include "forkedcall.h"
#include "forkedcontr.h"
#include "lstrings.h"
#include "lyxlib.h"
#include "filetools.h"
#include "os.h"
#include "debug.h"
#include "frontends/Timeout.h"

#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

using std::endl;

#ifndef CXX_GLOBAL_CSTD
using std::strerror;
#endif


Forkedcall::Forkedcall()
	: pid_(0), retval_(0)
{}


int Forkedcall::startscript(Starttype wait, string const & what)
{
	if (wait == Wait) {
		command_ = what;
		retval_  = 0;

		pid_ = generateChild();
		if (pid_ <= 0) { // child or fork failed.
			retval_ = 1;
		} else {
			retval_ = waitForChild();
		}

		return retval_;
	}

	// DontWait
	retval_ = startscript(what, SignalTypePtr());
	return retval_;
}


int Forkedcall::startscript(string const & what, SignalTypePtr signal)
{
	command_ = what;
	signal_  = signal;
	retval_  = 0;

	pid_ = generateChild();
	if (pid_ <= 0) { // child or fork failed.
		retval_ = 1;
		return retval_;
	}

	// Non-blocking execution.
	// Integrate into the Controller
	ForkedcallsController & contr = ForkedcallsController::get();
	contr.addCall(*this);

	return retval_;
}


void Forkedcall::emitSignal()
{
	if (signal_.get()) {
		signal_->emit(command_, pid_, retval_);
	}
}


namespace {

class Murder : public SigC::Object {
public:
	//
	static void killItDead(int secs, pid_t pid)
	{
		if (secs > 0) {
			new Murder(secs, pid);
		} else if (pid != 0) {
			lyx::kill(pid, SIGKILL);
		}
	}

	//
	void kill()
	{
		if (pid_ != 0) {
			lyx::kill(pid_, SIGKILL);
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
		timeout_->timeout.connect(SigC::slot(this, &Murder::kill));
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


void Forkedcall::kill(int tol)
{
	lyxerr << "Forkedcall::kill(" << tol << ")" << std::endl;
	if (pid() == 0) {
		lyxerr << "Can't kill non-existent process!" << endl;
		return;
	}

	int const tolerance = std::max(0, tol);
	if (tolerance == 0) {
		// Kill it dead NOW!
		Murder::killItDead(0, pid());

	} else {
		int ret = lyx::kill(pid(), SIGHUP);

		// The process is already dead if wait_for_death is false
		bool const wait_for_death = (ret == 0 && errno != ESRCH);

		if (wait_for_death) {
			Murder::killItDead(tolerance, pid());
		}
	}
}


// Wait for child process to finish. Returns returncode from child.
int Forkedcall::waitForChild() {
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


// generate child in background
pid_t Forkedcall::generateChild()
{
	const int MAX_ARGV = 255;
	char *syscmd = 0;
	char *argv[MAX_ARGV];

	string childcommand(command_); // copy
	bool more = true;
	string rest = split(command_, childcommand, ' ');

	int  index = 0;
	while (more) {
		childcommand = frontStrip(childcommand);
		if (syscmd == 0) {
			syscmd = new char[childcommand.length() + 1];
			childcommand.copy(syscmd, childcommand.length());
			syscmd[childcommand.length()] = '\0';
		}
		if (!childcommand.empty()) {
			char * tmp = new char[childcommand.length() + 1];
			childcommand.copy(tmp, childcommand.length());
			tmp[childcommand.length()] = '\0';
			argv[index++] = tmp;
		}

		// reinit
		more = !rest.empty();
		if (more)
			rest = split(rest, childcommand, ' ');
	}
	argv[index] = 0;

#ifndef __EMX__
	pid_t cpid = ::fork();
	if (cpid == 0) { // child
		execvp(syscmd, argv);
		// If something goes wrong, we end up here:
		lyxerr << "execvp failed: "
		       << strerror(errno) << endl;
	}
#else
	pid_t cpid = spawnvp(P_SESSION|P_DEFAULT|P_MINIMIZE|P_BACKGROUND,
			     syscmd, argv);
#endif

	if (cpid < 0) { // error
		lyxerr << "Could not fork: "
		       << strerror(errno) << endl;
	}

	return cpid;
}
