// -*- C++ -*-
/**
 * \file FormInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */
#ifndef FORMINCLUDE_H
#define FORMINCLUDE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlInclude;
struct FD_include;

/** This class provides an XForms implementation of the Include Dialog.
 */
class FormInclude : public FormCB<ControlInclude, FormDB<FD_include> > {
public:
	///
	FormInclude();
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

#endif
