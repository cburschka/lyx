// -*- C++ -*-
/**
 * \file xforms/FormIndex.h
 * Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMINDEX_H
#define FORMINDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlIndex;
struct FD_index;

/** This class provides an XForms implementation of the Index Dialog.
 */
class FormIndex : public FormCB<ControlIndex, FormDB<FD_index> > {
public:
	///
	FormIndex();
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMINDEX_H
