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
struct FD_ert;

/** This class provides an XForms implementation of the ERT
    Dialog.
 */
class FormERT
	: public FormCB<ControlERT, FormDB<FD_ert> > {
public:
	///
	FormERT(ControlERT &, Dialogs &);
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMERT_H
