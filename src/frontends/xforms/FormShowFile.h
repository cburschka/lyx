// -*- C++ -*-

/*
 * FormShowFile.h
 *
 * (C) 2001 LyX Team
 * \author Herbert Voss <voss@perce.de>
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
	FormShowFile(ControlShowFile &);

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply() {}
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMSHOWFILE_H
