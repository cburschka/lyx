// -*- C++ -*-
/*
 * \file FormBrowser.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
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
struct FD_form_browser;

class FormBrowser : public FormDB<FD_form_browser> {
public:
	///
	FormBrowser(ControlButtons &, string const &, bool allowResize=true);

private:
	/// Build the dialog.
	virtual void build();
	/// generated build function
	FD_form_browser * build_browser();
};

#endif // FORMBROWSER_H
