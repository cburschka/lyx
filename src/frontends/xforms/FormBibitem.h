// -*- C++ -*-
/**
 * \file FormBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMBIBITEM_H
#define FORMBIBITEM_H


#include "FormDialogView.h"

class ControlBibitem;
struct FD_bibitem;

/**
 * For bibliography entry editing
 */
class FormBibitem : public FormController<ControlBibitem, FormView<FD_bibitem> > {
public:
	///
	FormBibitem(Dialog &);
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
