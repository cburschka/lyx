// -*- C++ -*-
/*
 * FormBrowser.h
 *
 * (C) 2001 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMBROWSER_H
#define FORMBROWSER_H

#include <boost/smart_ptr.hpp>

#include "FormBaseDeprecated.h"
#include "xformsBC.h"

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
	///
	FormBrowser(LyXView *, Dialogs *, const string &);
protected:
	/// Update the dialog.
	virtual void update();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_browser> dialog_;
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
private:
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Filter the inputs on callback from xforms
	virtual bool input(FL_OBJECT *, long);
	/// Build the dialog
	virtual void build();

	/// generated build function
	FD_form_browser * build_browser();
	/// The ButtonController
	ButtonController<OkCancelPolicy, xformsBC> bc_;
};


inline
xformsBC & FormBrowser::bc()
{
	return bc_;
}
#endif
