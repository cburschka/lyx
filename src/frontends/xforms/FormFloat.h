// -*- C++ -*-
/**
 * \file FormFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMFLOAT_H
#define FORMFLOAT_H

#include "FormDialogView.h"
#include "RadioButtonGroup.h"

class ControlFloat;
struct FD_float;

/** This class provides an XForms implementation of the Float
    Dialog.
 */
class FormFloat : public FormController<ControlFloat, FormView<FD_float> > {
public:
	///
	FormFloat(Dialog &);
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// placement
	RadioButtonGroup placement_;
};

#endif // FORMFLOAT_H
