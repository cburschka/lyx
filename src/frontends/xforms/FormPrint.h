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

#include "FormBase.h"
#include "RadioButtonGroup.h"

#ifdef __GNUG__
#pragma interface
#endif

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
	///
	~FormPrint();

private:
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
	FD_form_print * dialog_;
	/// print target
	RadioButtonGroup target_;
	/// page order
	RadioButtonGroup order_;
	/// which pages
	RadioButtonGroup which_;
};

#endif
