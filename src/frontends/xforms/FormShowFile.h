// -*- C++ -*-
/**
 * \file FormShowFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMSHOWFILE_H
#define FORMSHOWFILE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBrowser.h"

class ControlShowFile;

/**
 * This class provides an XForms implementation of a dialog to browse through a
 * Help file.
 */
class FormShowFile : public FormCB<ControlShowFile, FormBrowser> {
public:
	///
	FormShowFile();

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMSHOWFILE_H
