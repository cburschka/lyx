// -*- C++ -*-
/**
 * FormLog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMLOG_H
#define FORMLOG_H

#include "FormBrowser.h"

namespace lyx {
namespace frontend {

class ControlLog;

/**
 * This class provides an XForms implementation of the LaTeX log dialog
 * for viewing the last LaTeX log file.
 */
class FormLog : public FormController<ControlLog, FormBrowser> {
public:
	///
	FormLog(Dialog &);

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();
};

} // namespace frontend
} // namespace lyx

#endif // FORMLOG_H
