#include <config.h>

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

SystemcallsSingletoncontroller::Startcontroller::
Startcontroller() {
}

SystemcallsSingletoncontroller::Startcontroller::
~Startcontroller() {
}

// Give reference to global controller-instance
// 
SystemcallsSingletoncontroller *
SystemcallsSingletoncontroller::Startcontroller::getController()
{ 
	if (!contr) { 
		// Create the global controller
                contr = new SystemcallsSingletoncontroller;
	}
	++refcount;
	return contr;
}
