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

#include <gnome--/main.h>

using std::endl;

extern bool finished;

void GUIRunTime::processEvents() 
{
	while(Gnome::Main::instance()->events_pending())
		Gnome::Main::instance()->iteration(FALSE);
}


void GUIRunTime::runTime()
{
	XEvent ev;
	while (!finished) {
		processEvents();
		if (fl_check_forms() == FL_EVENT) {
			lyxerr << "LyX: This shouldn't happen..." << endl;
			fl_XNextEvent(&ev);
		}
	}
}
