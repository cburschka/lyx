// -*- C++ -*-
/*
 * FormLog.h
 *
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMLOG_H
#define FORMLOG_H

#include "FormBase.h"
#include "FormBrowser.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class Dialogs;

/**
 * This class provides an XForms implementation of the LaTeX log dialog
 * for viewing the last LaTeX log file.
 */
class FormLog : public FormBrowser {
public:
	FormLog(LyXView *, Dialogs *);
	~FormLog();

private:
	/// Update the dialog.
	virtual void update();
};

#endif
