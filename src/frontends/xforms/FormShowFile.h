// -*- C++ -*-
/**
 * \file FormShowFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMSHOWFILE_H
#define FORMSHOWFILE_H

#include "FormBrowser.h"

namespace lyx {
namespace frontend {

class ControlShowFile;

/**
 * This class provides an XForms implementation of a dialog to browse through a
 * Help file.
 */
class FormShowFile : public FormController<ControlShowFile, FormBrowser> {
public:
	///
	FormShowFile(Dialog &);

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();
};

} // namespace frontend
} // namespace lyx

#endif // FORMSHOWFILE_H
