// -*- C++ -*-
/**
 * \file FormForks.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 */

#ifndef FORMFORKS_H
#define FORMFORKS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

struct FD_form_forks;
class ControlForks;

class FormForks : public FormCB<ControlForks, FormDB<FD_form_forks> > {
public:
	///
	FormForks(ControlForks &);

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
	/// tooltips
	string const getVerboseTooltip(FL_OBJECT const * ob) const;
	/// Fdesign generated method
	FD_form_forks * build_forks();

	ButtonPolicy::SMInput input_browser_children();
	ButtonPolicy::SMInput input_browser_kill();
	ButtonPolicy::SMInput input_button_all();
	ButtonPolicy::SMInput input_button_add();
	ButtonPolicy::SMInput input_button_remove();
};

#endif // FORMFORKS_H
