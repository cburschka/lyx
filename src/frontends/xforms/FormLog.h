// -*- C++ -*-
/*
 * FormLog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMLOG_H
#define FORMLOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBrowser.h"

class ControlLog;

/**
 * This class provides an XForms implementation of the LaTeX log dialog
 * for viewing the last LaTeX log file.
 */
class FormLog : public FormCB<ControlLog, FormBrowser> {
public:
	///
	FormLog(ControlLog &, Dialogs &);

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMLOG_H
