// -*- C++ -*-
/**
 * \file FormBibitem.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMBIBITEM_H
#define FORMBIBITEM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlBibitem;
struct FD_bibitem;

/**
 * For bibliography entry editing
 */
class FormBibitem : public FormCB<ControlBibitem, FormDB<FD_bibitem> > {
public:
	///
	FormBibitem(ControlBibitem &, Dialogs &);
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

#endif // FORMBIBITEM_H
