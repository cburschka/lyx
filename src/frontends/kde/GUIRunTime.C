/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GUIRunTime.h"
#include "debug.h"

#include <kapp.h>

#include FORMS_H_LOCATION

using std::endl;

extern bool finished;


void GUIRunTime::processEvents() 
{
	kapp->processEvents();
}


void GUIRunTime::runTime()
{
	// We still use xforms event handler as the main one...
	XEvent ev;
	while (!finished) {
		processEvents();
		if (fl_check_forms() == FL_EVENT) {
			lyxerr << "LyX: This shouldn't happen..." << endl;
			fl_XNextEvent(&ev);
		}
	}
}
