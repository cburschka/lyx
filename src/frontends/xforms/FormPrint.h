// -*- C++ -*-
/**
 * \file xforms/FormPrint.h
 * Copyright 1999-2001 Allan Rae
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMPRINT_H
#define FORMPRINT_H


#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"
#include "RadioButtonGroup.h"

class ControlPrint;
struct FD_print;

/** This class provides an XForms implementation of the FormPrint Dialog.
    The print dialog allows users to print their documents.
 */
class FormPrint : public FormCB<ControlPrint, FormDB<FD_print> > {
public:
	///
	FormPrint();
private:
	/// Apply from dialog
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update the dialog.
	virtual void update();

	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// print target
	RadioButtonGroup target_;
	/// all pages or from/to
	RadioButtonGroup all_pages_;
};

#endif // FORMPRINT_H
