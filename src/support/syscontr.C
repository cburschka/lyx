#include <config.h>

#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include "syscontr.h"
#include "syscall.h"
#include "debug.h"

#ifdef __GNUG__
#pragma implementation
#endif


using std::endl;

//
// Default constructor
//

SystemcallsSingletoncontroller::SystemcallsSingletoncontroller() 
{
       sysCalls = 0;
}

//
// Destructor
// 
// destroy structs for leaving program
// open question: should we stop childs here?
// Asger says no: I like to have my xdvi open after closing LyX. Maybe
// I want to print or something.

SystemcallsSingletoncontroller::~SystemcallsSingletoncontroller()
{
       ControlledCalls *next;
       while (sysCalls)
       {
               next = sysCalls->next;
               delete sysCalls;
	       sysCalls = next;
       }
       
}

//
// Add child process information into controlled list
//

void 
SystemcallsSingletoncontroller::addCall(Systemcalls const &newcall) {
       ControlledCalls * newCall = new ControlledCalls;
       if (newCall == 0) // sorry, no idea
               return;
       newCall->next = sysCalls;
       newCall->call = new Systemcalls(newcall);
       sysCalls = newCall;
}

// 
// Timer-call
// 
// Check list, if there is a stopped child. If yes, call-back.
//

void 
SystemcallsSingletoncontroller::timer() {
	lyxerr << "Tick" << endl;
	// check each entry of our list, if it's finished
        ControlledCalls *prev = 0;
	for (ControlledCalls *actCall= sysCalls; actCall; actCall= actCall->next)
	{
		pid_t pid= actCall->call->getpid();
		int stat_loc;
		int waitrpid = waitpid(pid, &stat_loc, WNOHANG);
		if (waitrpid == -1) {
			lyxerr << "LyX: Error waiting for child:" 
			       << strerror(errno) << endl;
		} else if (WIFEXITED(stat_loc) || WIFSIGNALED(stat_loc)) {
			if (WIFEXITED(stat_loc)) {
				// Ok, the return value goes into retval.
				actCall->call->setRetValue(WEXITSTATUS(stat_loc));
			} else {
				// Child died, so pretend it returned 1
				actCall->call->setRetValue(1);
			}
			// Callback and release
			actCall->call->callback();
			if (actCall == sysCalls) {
				sysCalls = actCall->next;
			} else {
				prev->next = actCall->next;
			}
			delete actCall;
			actCall = prev;
		} else if (WIFSTOPPED(stat_loc)) {
			lyxerr << "LyX: Child (pid: " << pid 
			       << ") stopped on signal "
			       << WSTOPSIG(stat_loc) 
			       << ". Waiting for child to finish." << endl;
		} else {
			lyxerr << "LyX: Something rotten happened while "
				"waiting for child " << pid << endl;
		}
		prev = actCall;
	}
}
