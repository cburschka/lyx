// -*- C++ -*-
/*
 * FormBrowser.h
 *
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMBROWSER_H
#define FORMBROWSER_H

#include "FormBase.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class Dialogs;
struct FD_form_browser;

/**
 * This class provides an XForms implementation of a read only
 * text browser.
 */
class FormBrowser : public FormBaseBD {
public:
	FormBrowser(LyXView *, Dialogs *, const string &);
	~FormBrowser();

protected:
	/// Update the dialog.
	virtual void update();

	/// Real GUI implementation.
	FD_form_browser * dialog_;

private:
	/// Pointer to the actual instantiation of the xform's form
	virtual FL_FORM * form() const;
	/// Filter the inputs on callback from xforms
	virtual bool input(FL_OBJECT *, long);
	/// Build the dialog
	virtual void build();

	/// generated build function
	FD_form_browser * build_browser();
};

#endif
