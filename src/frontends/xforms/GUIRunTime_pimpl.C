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

GUIRunTime::Pimpl::Pimpl()
{
}

GUIRunTime::Pimpl::~Pimpl() 
{
}

void GUIRunTime::Pimpl::processEvents() 
{
// this is disabled for now as we still need this ALWAYS in lyx_gui
// and if we enable this here we would do it 2 times well why not it
// doesn't really matter, does it? (Jug)
#if 1
    XEvent ev;

    if (fl_check_forms() == FL_EVENT) {
	lyxerr << "LyX: This shouldn't happen..." << endl;
	fl_XNextEvent(&ev);
    }
#endif
}
