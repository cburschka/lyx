// -*- C++ -*-
/**
 * \file FormBibitem.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 * \author John Levon
 */

#ifndef FORMBIBITEM_H
#define FORMBIBITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

#include "form_bibitem.h"

class ControlBibitem;

/**
 * For bibliography entry editing
 */
class FormBibitem : public FormBase2<ControlBibitem, FD_form_bibitem> {
public:
	///
	FormBibitem(ControlBibitem &);

	// Functions accessible to the Controller.

	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();

private:
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// Type definition from the fdesign produced header file.
	FD_form_bibitem * build_bibitem();
};

#endif // FORMBIBITEM_H
