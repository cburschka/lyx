// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *	    This file copyright 1999-2000
 *	    Allan Rae
 *======================================================*/
/* FormPrint.h
 * FormPrint Interface Class
 */

#ifndef FORMPRINT_H
#define FORMPRINT_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"
#include "RadioButtonGroup.h"
#include "xformsBC.h"

class LyXView;
class Dialogs;
struct FD_form_print;

/** This class provides an XForms implementation of the FormPrint Dialog.
    The print dialog allows users to print their documents.
 */
class FormPrint : public FormBaseBD {
public:
	/// #FormPrint x(LyXView ..., Dialogs ...);#
	FormPrint(LyXView *, Dialogs *);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Update the dialog.
	virtual void update();
	/// Apply from dialog
	virtual void apply();
	/// Filter the inputs
	virtual bool input(FL_OBJECT *, long);
	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Build the dialog
	virtual void build();
	/// Open the file browse dialog.
	void browse();

	///
	FD_form_print * build_print();
	
	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_print> dialog_;
	/// print target
	RadioButtonGroup target_;
	/// page order
	RadioButtonGroup order_;
	/// which pages
	RadioButtonGroup which_;
	/// The ButtonController
	ButtonController<OkApplyCancelPolicy, xformsBC> bc_;
};


inline
xformsBC & FormPrint::bc()
{
	return bc_;
}
#endif
