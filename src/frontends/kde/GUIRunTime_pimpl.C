/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright (C) 2000 The LyX Team.
 *
 *           @author Jürgen Vigna
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GUIRunTime_pimpl.h"
#include "debug.h"

#include <kapp.h>

#include FORMS_H_LOCATION

using std::endl;

extern bool finished;


void GUIRunTime::Pimpl::processEvents() 
{
    kapp->processEvents();
}


void GUIRunTime::Pimpl::runTime()
{
	// We still use xforms event handler as te main one...
	XEvent ev;
	while (!finished) {
		Pimpl::processEvents();
		if (fl_check_forms() == FL_EVENT) {
			lyxerr << "LyX: This shouldn't happen..." << endl;
			fl_XNextEvent(&ev);
		}
	}
}

