/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 *	    This file copyright 1999-2000
 *	    Allan Rae
 *======================================================
 *
 * \file FormPrint.h
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef FORMPRINT_H
#define FORMPRINT_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"
#include "RadioButtonGroup.h"

class ControlPrint;
struct FD_form_print;

/** This class provides an XForms implementation of the FormPrint Dialog.
    The print dialog allows users to print their documents.
 */
class FormPrint : public FormCB<ControlPrint, FormDB<FD_form_print> > {
public:
	/// 
	FormPrint(ControlPrint &);

private:
	/// Apply from dialog
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update the dialog.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// Fdesign generated method
	FD_form_print * build_print();

	/// print target
	RadioButtonGroup target_;
	/// page order
	RadioButtonGroup order_;
	/// which pages
	RadioButtonGroup which_;
};

#endif // FORMPRINT_H
