// -*- C++ -*-
/**
 * FormLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
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
	FormLog();

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMLOG_H
