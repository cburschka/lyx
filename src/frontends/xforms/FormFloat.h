// -*- C++ -*-
/**
 * \file FormFloat.h
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMFLOAT_H
#define FORMFLOAT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlFloat;
struct FD_float;

/** This class provides an XForms implementation of the Float
    Dialog.
 */
class FormFloat : public FormCB<ControlFloat, FormDB<FD_float> > {
public:
	///
	FormFloat();
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif // FORMFLOAT_H
