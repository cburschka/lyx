/**
 * \file FormFloat.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlFloat.h"
#include "FormFloat.h"
#include "forms/form_float.h"
#include "support/lstrings.h"
#include "xforms_helpers.h"
#include FORMS_H_LOCATION

typedef FormCB<ControlFloat, FormDB<FD_float> > base_class;

FormFloat::FormFloat(ControlFloat & c, Dialogs & d)
	: base_class(c, d, _("Float Options"))
{}


// FIX: Needs to be implemented. (Lgb)
// A way to set to float default is missing.
// A way to set "force[!]" is missing.

void FormFloat::build()
{
	dialog_.reset(build_float(this));

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->check_top);
	bc().addReadOnly(dialog_->check_bottom);
	bc().addReadOnly(dialog_->check_page);
	bc().addReadOnly(dialog_->check_here);
	bc().addReadOnly(dialog_->check_here_definitely);
	bc().addReadOnly(dialog_->check_wide);
}


void FormFloat::apply()
{
	string placement;
	if (fl_get_button(dialog_->check_here_definitely)) {
		placement += "H";
	} else {
		if (fl_get_button(dialog_->check_top)) {
			placement += "t";
		}
		if (fl_get_button(dialog_->check_bottom)) {
			placement += "b";
		}
		if (fl_get_button(dialog_->check_page)) {
			placement += "p";
		}
		if (fl_get_button(dialog_->check_here)) {
			placement += "h";
		}
	}
	controller().params().placement = placement;
	controller().params().wide = fl_get_button(dialog_->check_wide);
}


void FormFloat::update()
{
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool here_definitely = false;

	string placement(controller().params().placement);

	if (contains(placement, "H")) {
		here_definitely = true;
	} else {
		if (contains(placement, "t")) {
			top = true;
		}
		if (contains(placement, "b")) {
			bottom = true;
		}
		if (contains(placement, "p")) {
			page = true;
		}
		if (contains(placement, "h")) {
			here = true;
		}
	}
	fl_set_button(dialog_->check_top, top);
	fl_set_button(dialog_->check_bottom, bottom);
	fl_set_button(dialog_->check_page, page);
	fl_set_button(dialog_->check_here, here);
	fl_set_button(dialog_->check_here_definitely, here_definitely);
	setEnabled(dialog_->check_here_definitely, !controller().params().wide);
	fl_set_button(dialog_->check_wide, controller().params().wide);
}


ButtonPolicy::SMInput FormFloat::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->check_here_definitely) {
		if (fl_get_button(dialog_->check_here_definitely)) {
			fl_set_button(dialog_->check_top,    false);
			fl_set_button(dialog_->check_bottom, false);
			fl_set_button(dialog_->check_page,   false);
			fl_set_button(dialog_->check_here,   false);
		}
	} else {
		if (fl_get_button(dialog_->check_here_definitely)) {
			fl_set_button(dialog_->check_here_definitely, false);
		}
	}
	if (ob == dialog_->check_wide) {
		if (fl_get_button(dialog_->check_wide)) {
			fl_set_button(dialog_->check_here_definitely, false);
			setEnabled(dialog_->check_here_definitely, false);
		}
		else
			setEnabled(dialog_->check_here_definitely, true);
	}

	return ButtonPolicy::SMI_VALID;
}
