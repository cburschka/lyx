// -*- C++ -*-
/**
 * \file FormBibitem.h
 * Read the file COPYING
 *
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
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
	FormBibitem();
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
