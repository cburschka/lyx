#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "syscontr.h"


//----------------------------------------------------------------------
// Ensure, that only one controller exist inside process
//----------------------------------------------------------------------

// global controller variable
SystemcallsSingletoncontroller *
SystemcallsSingletoncontroller::Startcontroller::
contr = 0;

// reference count
int SystemcallsSingletoncontroller::Startcontroller::
refcount = 0;

// default constructor. 
// Nothing to do at moment.
SystemcallsSingletoncontroller::Startcontroller::
Startcontroller() 
{
}

// default destructor.
// Nothing to do at moment
SystemcallsSingletoncontroller::Startcontroller::
~Startcontroller() 
{
}

// Give reference to global controller-instance
// 
SystemcallsSingletoncontroller *
SystemcallsSingletoncontroller::Startcontroller::
GetController()
{ 
	if (! contr) 
		{ // generate the global controller
                        contr = new SystemcallsSingletoncontroller;
		}
	refcount++;
	return contr;
}
