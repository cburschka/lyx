/**
 * \file FormFloat.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 * \author Juergen Spitzmueller j.spitzmueller@gmx.de
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


void FormFloat::build()
{
	dialog_.reset(build_float(this));

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->check_default);
	bc().addReadOnly(dialog_->check_top);
	bc().addReadOnly(dialog_->check_bottom);
	bc().addReadOnly(dialog_->check_page);
	bc().addReadOnly(dialog_->check_here);
	bc().addReadOnly(dialog_->check_force);
	bc().addReadOnly(dialog_->check_here_definitely);
	bc().addReadOnly(dialog_->check_wide);
}


void FormFloat::apply()
{
	string placement;
	if (fl_get_button(dialog_->check_here_definitely)) {
		placement += "H";
	} else {
		if (fl_get_button(dialog_->check_force)) {
			placement += "!";
		}
		if (fl_get_button(dialog_->check_here)) {
			placement += "h";
		}
		if (fl_get_button(dialog_->check_top)) {
			placement += "t";
		}
		if (fl_get_button(dialog_->check_bottom)) {
			placement += "b";
		}
		if (fl_get_button(dialog_->check_page)) {
			placement += "p";
		}

	}
	controller().params().placement = placement;
	controller().params().wide = fl_get_button(dialog_->check_wide);
}


void FormFloat::update()
{
	bool def_placement = false;
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool force = false;
	bool here_definitely = false;

	string placement(controller().params().placement);

	if (placement.empty()) {
		def_placement = true;

	} else if (contains(placement, "H")) {
		here_definitely = true;

	} else {
		if (contains(placement, "!")) {
			force = true;
		}
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
	fl_set_button(dialog_->check_default, def_placement);
	fl_set_button(dialog_->check_top, top);
	fl_set_button(dialog_->check_bottom, bottom);
	fl_set_button(dialog_->check_page, page);
	fl_set_button(dialog_->check_here, here);
	fl_set_button(dialog_->check_force, force);
	fl_set_button(dialog_->check_here_definitely, here_definitely);
	setEnabled(dialog_->check_here_definitely, !controller().params().wide 
			&& !def_placement);
	if (controller().params().wide) {
			fl_set_button(dialog_->check_here, false);
			fl_set_button(dialog_->check_bottom, false);
	}
	setEnabled(dialog_->check_here, !controller().params().wide && !def_placement);
	setEnabled(dialog_->check_bottom, !controller().params().wide && !def_placement);
	fl_set_button(dialog_->check_wide, controller().params().wide);
	setEnabled(dialog_->check_top, !def_placement);
	setEnabled(dialog_->check_page, !def_placement);
	setEnabled(dialog_->check_force, !def_placement);
}


ButtonPolicy::SMInput FormFloat::input(FL_OBJECT * ob, long)
{
	bool const def_place = fl_get_button(dialog_->check_default);
	if (ob == dialog_->check_default) {
		if (def_place) {
			fl_set_button(dialog_->check_top, false);
			fl_set_button(dialog_->check_bottom,  false);
			fl_set_button(dialog_->check_page, false);
			fl_set_button(dialog_->check_here, false);
			fl_set_button(dialog_->check_force, false);
			fl_set_button(dialog_->check_here_definitely, false);
			}
		setEnabled(dialog_->check_top, !def_place);
		setEnabled(dialog_->check_bottom, !def_place);
		setEnabled(dialog_->check_page, !def_place);
		setEnabled(dialog_->check_here, !def_place);
		setEnabled(dialog_->check_force, !def_place);
		setEnabled(dialog_->check_here_definitely, !def_place);

	} else if (ob == dialog_->check_here_definitely) {
		if (fl_get_button(dialog_->check_here_definitely)) {
			fl_set_button(dialog_->check_top,    false);
			fl_set_button(dialog_->check_bottom, false);
			fl_set_button(dialog_->check_page,   false);
			fl_set_button(dialog_->check_here,   false);
			fl_set_button(dialog_->check_force,   false);
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
			fl_set_button(dialog_->check_here, false);
			setEnabled(dialog_->check_here, false);
			fl_set_button(dialog_->check_bottom, false);
			setEnabled(dialog_->check_bottom, false);
		
		} else {
			setEnabled(dialog_->check_here_definitely, true);
			setEnabled(dialog_->check_here, true);
			setEnabled(dialog_->check_bottom, true);
		}
	}

	return ButtonPolicy::SMI_VALID;
}
