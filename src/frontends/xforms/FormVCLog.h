// -*- C++ -*-
/*
 * FormVCLog.h
 *
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMVCLOG_H
#define FORMVCLOG_H

#include "FormBase.h"
#include "FormBrowser.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class Dialogs;

/**
 * This class provides an XForms implementation of the Version Control
 * log viewer
 */
class FormVCLog : public FormBrowser {
public:
	FormVCLog(LyXView *, Dialogs *);
	~FormVCLog();

private:
	/// Update the dialog.
	virtual void update();
};

#endif
