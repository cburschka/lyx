// -*- C++ -*-
/**
 * \file FormFloat.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 */

#ifndef FORMFLOAT_H
#define FORMFLOAT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlFloat;
struct FD_form_float;

/** This class provides an XForms implementation of the Float
    Dialog.
 */
class FormFloat : public FormCB<ControlFloat, FormDB<FD_form_float> > {
public:
	///
	FormFloat(ControlFloat &);

private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// Fdesign generated method
	FD_form_float * build_float();
};

#endif // FORMFLOAT_H
