// -*- C++ -*-
/**
 * \file FormVCLog.h
 * Read the file COPYING
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMVCLOG_H
#define FORMVCLOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBrowser.h"

class ControlVCLog;

/**
 * This class provides an XForms implementation of the Version Control
 * log viewer
 */
class FormVCLog : public FormCB<ControlVCLog, FormBrowser> {
public:
	///
	FormVCLog();

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMVCLOG_H
