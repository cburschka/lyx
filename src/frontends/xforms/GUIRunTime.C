/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION
#include "GUIRunTime.h"
#include "debug.h"

// I keep these here so that it will be processed as early in
// the compilation process as possible.
#if !defined(FL_REVISION) || FL_REVISION < 88 || FL_VERSION != 0
#error LyX will not compile with this version of XForms.\
       Please get version 0.89.\
       If you want to try to compile anyway, delete this test in src/frontends/xforms/GUIRunTime.C.
#endif

using std::endl;

extern bool finished;

static int const xforms_include_version = FL_INCLUDE_VERSION;

int GUIRunTime::initApplication(int argc, char * argv[])
{
	// Check the XForms version in the forms.h header against
	// the one in the libforms. If they don't match quit the
	// execution of LyX. Better with a clean fast exit than
	// a strange segfault later.
	// I realize that this check have to be moved when we
	// support several toolkits, but IMO all the toolkits
	// should try to have the same kind of check. This could
	// be done by having a CheckHeaderAndLib function in
	// all the toolkit implementations, this function is
	// responsible for notifing the user.
	// if (!CheckHeaderAndLib()) {
	//         // header vs. lib version failed
	//         return 1;
	// }
	int xforms_lib_version = fl_library_version(0, 0);
	if (xforms_include_version != xforms_lib_version) {
		cerr << "You are either running LyX with wrong "
			"version of a dynamic XForms library\n"
			"or you have build LyX with conflicting header "
			"and library (different\n"
			"versions of XForms. Sorry but there is no point "
			"in continuing executing LyX!" << endl;
		return 1;
	}
	return 0;
}

void GUIRunTime::processEvents() 
{
    XEvent ev;

    if (fl_do_forms() == FL_EVENT) {
	lyxerr << "LyX: This shouldn't happen..." << endl;
	fl_XNextEvent(&ev);
    }
}


void GUIRunTime::runTime()
{
	XEvent ev;
	while (!finished) {
		if (fl_check_forms() == FL_EVENT) {
			lyxerr << "LyX: This shouldn't happen..." << endl;
			fl_XNextEvent(&ev);
		}
	}
}

