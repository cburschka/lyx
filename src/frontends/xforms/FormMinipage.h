// -*- C++ -*-
/**
 * \file FormMinipage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMMINIPAGE_H
#define FORMMINIPAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlMinipage;
struct FD_minipage;

/** This class provides an XForms implementation of the Minipage
    Dialog.
 */
class FormMinipage
	: public FormCB<ControlMinipage, FormDB<FD_minipage> > {
public:
	///
	FormMinipage();
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

#endif // FORMMINIPAGE_H
