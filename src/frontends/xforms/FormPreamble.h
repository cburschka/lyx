/**
 * \file FormPreamble.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef FORMPREAMBLE_H
#define FORMPREAMBLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlPreamble;
struct FD_form_preamble;

/** This class provides an XForms implementation of the Preamble Dialog.
 */
class FormPreamble : public FormCB<ControlPreamble, FormDB<FD_form_preamble> > {
public:
	///
	FormPreamble(ControlPreamble &);
private:
	/// Apply from dialog 
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update the dialog
	virtual void update();
   
	/// Fdesign generated method
	FD_form_preamble * build_preamble();
};


#endif // FORMPREAMBLE_H
