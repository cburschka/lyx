// -*- C++ -*-
/**
 * \file FormError.h
 * Copyright 2000-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMERROR_H
#define FORMERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlError;
struct FD_form_error;

/** This class provides an XForms implementation of the Error Dialog.
 */
class FormError : public FormCB<ControlError, FormDB<FD_form_error> > {
public:
	/// Constructor
	FormError(ControlError &);

private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();

	/// Fdesign generated method
	FD_form_error * build_error();
};

#endif // FORMERROR_H
