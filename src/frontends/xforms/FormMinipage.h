// -*- C++ -*-
/**
 * \file FormMinipage.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#ifndef FORMMINIPAGE_H
#define FORMMINIPAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlMinipage;
struct FD_form_minipage;

/** This class provides an XForms implementation of the Minipage
    Dialog.
 */
class FormMinipage
	: public FormCB<ControlMinipage, FormDB<FD_form_minipage> > {
public:
	///
	FormMinipage(ControlMinipage &);
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
	FD_form_minipage * build_minipage();
};

#endif // FORMMINIPAGE_H
