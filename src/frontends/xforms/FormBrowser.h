// -*- C++ -*-
/*
 * \file FormBrowser.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifndef FORMBROWSER_H
#define FORMBROWSER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

/**
 * This class provides an XForms implementation of a read only
 * text browser.
 */
struct FD_browser;

class FormBrowser : public FormDB<FD_browser> {
public:
	///
	FormBrowser(string const &, bool allowResize = true);
private:
	/// Build the dialog.
	virtual void build();
};

#endif // FORMBROWSER_H
