// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *======================================================*/
/* FormMinipage.h
 * FormMinipage Interface Class
 */

#ifndef FORMMINIPAGE_H
#define FORMMINIPAGE_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormInset.h"
#include "xformsBC.h"

class LyXView;
class Dialogs;
class InsetMinipage;
struct FD_form_minipage;

/** This class provides an XForms implementation of the FormMinipage
    Dialog.
 */
class FormMinipage : public FormInset {
public:
	/// #FormMinipage x(LyXView ..., Dialogs ...);#
	FormMinipage(LyXView *, Dialogs *);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Connect signals etc.
	virtual void connect();

	/// Slot launching dialog to an existing inset
	void showInset(InsetMinipage *);
	/// Slot launching dialog to an existing inset
	void updateInset(InsetMinipage *);
	/// Apply from dialog
	virtual void apply();
	/// Update dialog before showing it
	virtual void update();
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Build the dialog
	virtual void build();

	///
	FD_form_minipage * build_minipage();
	
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_minipage> dialog_;
	/// The ButtonController
	ButtonController<OkApplyCancelReadOnlyPolicy, xformsBC> bc_;

	/// pointer to the inset passed through showInset
	InsetMinipage * inset_;
};


inline
xformsBC & FormMinipage::bc()
{
	return bc_;
}
#endif
