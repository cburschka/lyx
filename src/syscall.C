#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "syscall.h"
#include "syscontr.h"

//----------------------------------------------------------------------
// Class, which controlls a system-call
//----------------------------------------------------------------------

// constructor
Systemcalls::Systemcalls()
{
	pid = (pid_t) 0; // yet no child
}

// constructor
// 
// starts child
Systemcalls::Systemcalls(Starttype how, LString what, Callbackfct cback)
{
	start   = how;
	command = what;
	cbk     = cback;
	pid     = (pid_t) 0; // no child yet
	retval  = 0;
	Startscript();
}

// destructor
// not yet implemented (?)
Systemcalls::~Systemcalls()
{
}

// Start a childprocess
// 
// if child runs in background, add information to global controller.

int Systemcalls::Startscript()
{
	retval = 0;
	switch (start) {
	case System: 
		retval = system(command.c_str());
		Callback();
		break;
	case Wait:   
		pid = Fork();
		if (pid>0) { // Fork succesful. Wait for child
			waitForChild();
			Callback();
		} else
			retval = 1;
		break;
	case DontWait:
		pid=Fork();
		if (pid>0) {
			// Now integrate into Controller
			SystemcallsSingletoncontroller::Startcontroller starter;
			SystemcallsSingletoncontroller *contr=
				starter.GetController();
			// Add this to contr
			contr->AddCall(*this);
		} else
			retval = 1;
		break;
		//default:  // error();
		//break;
	}
	return retval;
}


// Wait for child process to finish. Returns returncode from child.
void Systemcalls::waitForChild()
{
	// We'll pretend that the child returns 1 on all errorconditions.
	retval = 1;
	int status;
	bool wait = true;
	while (wait) {
		pid_t waitrpid = waitpid(pid, &status, WUNTRACED);
		if (waitrpid == -1) {
			perror("LyX: Error waiting for child");
			wait = false;
		} else if (WIFEXITED(status)) {
			// Child exited normally. Update return value.
			retval = WEXITSTATUS(status);
			wait = false;
		} else if (WIFSIGNALED(status)) {
			fprintf(stderr,"LyX: Child didn't catch signal %d "
				"and died. Too bad.\n", WTERMSIG(status));
			wait = false;
		} else if (WIFSTOPPED(status)) {
			fprintf(stderr,"LyX: Child (pid: %ld) stopped on "
				"signal %d. Waiting for child to finish.\n", 
				(long) pid, WSTOPSIG(status));
		} else {
			fprintf(stderr,"LyX: Something rotten happened while "
				"waiting for child %ld\n", (long) pid);
			wait = false;
		}
	}
}


// generate child in background

pid_t Systemcalls::Fork()
{
	pid_t cpid=fork();
	if (cpid == 0) { // child
		LString childcommand(command); // copy
		LString rest = command.split(childcommand, ' ');
		const int MAX_ARGV = 255;
		char *syscmd = NULL; 
		char *argv[MAX_ARGV];
		int  index = 0;
		bool Abbruch;
		do {
			if (syscmd == NULL) {
				syscmd = childcommand.copy();
			}
			argv[index++] = childcommand.copy();
			// reinit
			Abbruch = !rest.empty();
			if (Abbruch) 
				rest = rest.split(childcommand, ' ');
		} while (Abbruch);
		argv[index] = NULL;
		// replace by command. Expand using PATH-environment-var.
		execvp(syscmd, argv);
		// If something goes wrong, we end up here:
		perror("LyX: execvp failed");
	} else if (cpid < 0) { // error
		perror("LyX: Could not fork");
	} else { // parent
		return cpid;
	}
	return 0;
}


// Reuse of instance

int Systemcalls::Startscript(Starttype how, LString what, Callbackfct cback)
{
	start   = how;
	command = what;
	cbk     = cback;
	pid     = (pid_t) 0; // yet no child
	retval	= 0;
        return Startscript();
}



//
// Mini-Test-environment for script-classes
//
#ifdef TEST_MAIN
#include <stdio.h>


int SimulateTimer;
void back(LString cmd, int retval)
{
	printf("Done: %s gave %d\n", cmd.c_str(), retval);
	SimulateTimer = 0;
}


int main(int, char**)
{
	
	SystemcallsSingletoncontroller::Startcontroller starter; 
	SystemcallsSingletoncontroller *contr=starter.GetController();
	
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
