// -*- C++ -*-
/**
 * \file xforms/FormERT.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#ifndef FORMERT_H
#define FORMERT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlERT;
struct FD_form_ert;

/** This class provides an XForms implementation of the ERT
    Dialog.
 */
class FormERT
	: public FormCB<ControlERT, FormDB<FD_form_ert> > {
public:
	///
	FormERT(ControlERT &);
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();

	/// Fdesign generated method
	FD_form_ert * build_ert();
};

#endif // FORMERT_H
