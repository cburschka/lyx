// -*- C++ -*-
/**
 * \file FormForks.h
 * Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMFORKS_H
#define FORMFORKS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlForks;
struct FD_forks;

class FormForks : public FormCB<ControlForks, FormDB<FD_forks> > {
public:
	///
	FormForks();

	/// preemptive handler for feedback messages
	void feedbackCB(FL_OBJECT *, int);

private:
	/// Return the list of PIDs to kill to the controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update the dialog.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	ButtonPolicy::SMInput input_browser_children();
	ButtonPolicy::SMInput input_browser_kill();
	ButtonPolicy::SMInput input_button_all();
	ButtonPolicy::SMInput input_button_add();
	ButtonPolicy::SMInput input_button_remove();
};

#endif // FORMFORKS_H
