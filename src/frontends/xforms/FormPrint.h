// -*- C++ -*-
/**
 * \file xforms/FormPrint.h
 * Copyright 2002 the LyX Team
 * Copyright 1999-2001 Allan Rae
 * Read the file COPYING
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming <leeming@lyx.org>
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
	/// page order
	RadioButtonGroup order_;
	/// which pages
	RadioButtonGroup which_;
};

#endif // FORMPRINT_H
