// -*- C++ -*-
/**
 * \file FormPreamble.h
 * See the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMPREAMBLE_H
#define FORMPREAMBLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlPreamble;
struct FD_preamble;

/** This class provides an XForms implementation of the Preamble Dialog.
 */
class FormPreamble : public FormCB<ControlPreamble, FormDB<FD_preamble> > {
public:
	///
	FormPreamble();
private:
	/// Apply from dialog
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update the dialog
	virtual void update();
};


#endif // FORMPREAMBLE_H
