#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include "debug.h"
#include "syscall.h"
#include "syscontr.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/filetools.h"
#include "support/os.h"

using std::endl;

#ifndef CXX_GLOBAL_CSTD
using std::strerror;
#endif


Systemcalls::Systemcalls() {
	pid = 0; // No child yet
}

Systemcalls::Systemcalls(Starttype how, string const & what, Callbackfct cback)
{
	startscript(how, what, cback);
}

Systemcalls::~Systemcalls() {
#if 0
	// If the child is alive, we have to brutally kill it
	if (getpid() != 0) {
		lyx::kill(getpid(), SIGKILL);
	}
#endif
}

// Start a childprocess
// 
// if child runs in background, add information to global controller.

int Systemcalls::startscript() {
	retval = 0;
	switch (start) {
	case System: 
	case SystemDontWait:
		retval = ::system(command.c_str());
		callback();
		break;
	case Wait:   
		pid = fork();
		if (pid>0) { // Fork succesful. Wait for child
			waitForChild();
			callback();
		} else
			retval = 1;
		break;
	case DontWait:
		pid = fork();
		if (pid>0) {
			// Now integrate into Controller
			SystemcallsSingletoncontroller::Startcontroller starter;
			SystemcallsSingletoncontroller * contr = starter.getController();
			// Add this to controller
			contr->addCall(*this);
		} else
			retval = 1;
		break;
	}
	return retval;
}

void Systemcalls::kill(int /*tolerance*/) {
	if (getpid() == 0) {
		lyxerr << "LyX: Can't kill non-existing process." << endl;
		return;
	}
	int ret = lyx::kill(getpid(), SIGHUP);
	bool wait_for_death = true;
	if (ret != 0) {
		if (errno == ESRCH) {
			// The process is already dead!
			wait_for_death = false;
		} else {
			// Something is rotten - maybe we lost permissions?
		}
	}
	if (wait_for_death) {
		// Here, we should add the PID to a list of
		// waiting processes to kill if they are not
		// dead within tolerance seconds

		// CHECK Implement this using the timer of
		// the singleton systemcontroller (Asger)

	}
}


// Wait for child process to finish. Returns returncode from child.
void Systemcalls::waitForChild() {
	// We'll pretend that the child returns 1 on all errorconditions.
	retval = 1;
	int status;
	bool wait = true;
	while (wait) {
		pid_t waitrpid = waitpid(pid, &status, WUNTRACED);
		if (waitrpid == -1) {
			lyxerr << "LyX: Error waiting for child:"
			       << strerror(errno) << endl;
			wait = false;
		} else if (WIFEXITED(status)) {
			// Child exited normally. Update return value.
			retval = WEXITSTATUS(status);
			wait = false;
		} else if (WIFSIGNALED(status)) {
			lyxerr << "LyX: Child didn't catch signal "
			       << WTERMSIG(status) 
			       << "and died. Too bad." << endl;
			wait = false;
		} else if (WIFSTOPPED(status)) {
			lyxerr << "LyX: Child (pid: " << pid 
			       << ") stopped on signal "
			       << WSTOPSIG(status) 
			       << ". Waiting for child to finish." << endl;
		} else {
			lyxerr << "LyX: Something rotten happened while "
				"waiting for child " << pid << endl;
			wait = false;
		}
	}
}


// generate child in background

pid_t Systemcalls::fork()
{
	#ifndef __EMX__
	pid_t cpid= ::fork();
	if (cpid == 0) { // child
	#endif
		// TODO: Consider doing all of this before the fork, otherwise me
		// might have troubles with multi-threaded access. (Baruch 20010228)
		string childcommand(command); // copy
		string rest = split(command, childcommand, ' ');
		const int MAX_ARGV = 255;
		char *syscmd = 0; 
		char *argv[MAX_ARGV];
		int  index = 0;
		bool more = true;
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
		// replace by command. Expand using PATH-environment-var.
#ifndef __EMX__
		execvp(syscmd, argv);
		// If something goes wrong, we end up here:
		lyxerr << "LyX: execvp failed: "
		       << strerror(errno) << endl;
	} else if (cpid < 0) { // error
#else
	pid_t cpid = spawnvp(P_SESSION|P_DEFAULT|P_MINIMIZE|P_BACKGROUND, syscmd, argv);
	if (cpid < 0) { // error
#endif
		lyxerr << "LyX: Could not fork: "
		       << strerror(errno) << endl;
	} else { // parent
		return cpid;
	}
	return 0;
}


// Reuse of instance

int Systemcalls::startscript(Starttype how, string const & what, 
                             Callbackfct cback)
{
	start   = how;
	command = what;
	cbk     = cback;
	pid     = static_cast<pid_t>(0); // yet no child
	retval	= 0;

	if (how == SystemDontWait) {
		if (os::shell() == os::UNIX) {
			command += " &";
		} else {
			command = "start /min/n " + command;
		}
	}

        return startscript();
}



//
// Mini-Test-environment for script-classes
//
#ifdef TEST_MAIN
#include <stdio.h>


int SimulateTimer;
void back(string cmd, int retval)
{
	::printf("Done: %s gave %d\n", cmd.c_str(), retval);
	SimulateTimer = 0;
}


int main(int, char **)
{
	
	SystemcallsSingletoncontroller::Startcontroller starter; 
	SystemcallsSingletoncontroller *contr= starter.GetController();
	
	Systemcalls one(Systemcalls::System, "ls -ltag", back);
	Systemcalls two(Systemcalls::Wait, "ls -ltag", back);
	SimulateTimer = 1;
	Systemcalls three(Systemcalls::DontWait , "ls -ltag", back);
	// Simulation of timer
	while (SimulateTimer)
		{
			sleep(1);
			contr->Timer();
		}
}
#endif
