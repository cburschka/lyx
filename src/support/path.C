#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "path.h"

int Path::pop()
{
	if (popped_) {
		// should throw an exception
		// throw logical_error();
		// The use of WriteFSAlerrt makes this impossible
		// to inline.
		//Alert::err_alert(_("Error: Dir already popped: "),
		//	     pushedDir_);
		return 0;
	}
	if (lyx::chdir(pushedDir_)) {
		// should throw an exception
		// throw DirChangeError();
		// The use of Alert::err_alert makes this impossible
		// to inline.
		//Alert::err_alert(
		//	_("Error: Could not change to directory: "), 
		//	pushedDir_);
	}
	popped_ = true;
	return 0;
}

