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

#include FORMS_H_LOCATION
#include "GUIRunTime_pimpl.h"
#include "debug.h"

using std::endl;

extern bool finished;


void GUIRunTime::Pimpl::processEvents() 
{
    XEvent ev;

    if (fl_do_forms() == FL_EVENT) {
	lyxerr << "LyX: This shouldn't happen..." << endl;
	fl_XNextEvent(&ev);
    }
}


void GUIRunTime::Pimpl::runTime()
{
	XEvent ev;
	while (!finished) {
		if (fl_check_forms() == FL_EVENT) {
			lyxerr << "LyX: This shouldn't appen..." << endl;
			fl_XNextEvent(&ev);
		}
	}
}

