// -*- C++ -*-
/**
 * \file FormBibitem.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifndef FORMBIBITEM_H
#define FORMBIBITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlBibitem;
struct FD_form_bibitem;

/**
 * For bibliography entry editing
 */
class FormBibitem : public FormCB<ControlBibitem, FormDB<FD_form_bibitem> > {
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
