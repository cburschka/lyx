#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "syscontr.h"
#include "syscall.h"

#ifdef __GNUG__
#pragma implementation
#endif

//----------------------------------------------------------------------
// Controller-Implementation
//----------------------------------------------------------------------

//
// default contstructor
//

SystemcallsSingletoncontroller::SystemcallsSingletoncontroller() 
{
       SysCalls = 0;
}

//
// destructor
// 
// destroy structs for leaving program
// open question: should we stop here childs?
// Asger says no: I like to have my xdvi open after closing LyX. Maybe
// I want to print or something.

SystemcallsSingletoncontroller::~SystemcallsSingletoncontroller()
{
       ControlledCalls *next;
       while (SysCalls)
       {
               next = SysCalls->next;
               delete SysCalls;
               SysCalls = next;
       }
       
}

//
// Add childprocessinformation into controlled list
//

void 
SystemcallsSingletoncontroller::AddCall(Systemcalls const &newcall)
{
// not yet implemented
       ControlledCalls *newCall = new ControlledCalls;
       if (newCall == 0) // sorry, no idea
               return;
       newCall->next = SysCalls;
       newCall->call = new Systemcalls(newcall);
       SysCalls = newCall;
}

// 
// Timer-call
// 
// Check list, if there is a stopped child. If yes, call-back.
//

void 
SystemcallsSingletoncontroller::Timer()
{
	// check each entry of our list, if it's finished
        ControlledCalls *prev = 0;
	for (ControlledCalls *actCall=SysCalls; actCall; actCall=actCall->next) 
		{
			pid_t pid=actCall->call->Getpid();
			int stat_loc;
			waitpid(pid, &stat_loc, WNOHANG);
			if (WIFEXITED(stat_loc) || WIFSIGNALED(stat_loc)) {
				// Ok, the return value goes into retval.
				if (WIFEXITED(stat_loc)) {
					actCall->call->setRetValue(WEXITSTATUS(stat_loc));
				} else {
					// Child died, so pretend it returned 1
					actCall->call->setRetValue(1);
				}
				// callback and release
				actCall->call->Callback();
				if (actCall == SysCalls) {
					SysCalls = actCall->next;
				} else {
					prev->next = actCall->next;
				}
				delete actCall;
				actCall = prev;
			}
			prev = actCall;
		}
}
