/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormFloat.C
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlFloat.h"
#include "FormFloat.h"
#include "form_float.h"
#include "support/lstrings.h"

typedef FormCB<ControlFloat, FormDB<FD_form_float> > base_class;

FormFloat::FormFloat(ControlFloat & c)
	: base_class(c, _("Float Options"))
{}


// FIX: Needs to be implemented. (Lgb)
// A way to set to float default is missing.
// A way to set "force[!]" is missing.
// Also there are two groups of buttons [Here] and [top,bottom,page,here],
// is is not really possible to choose from both groups. So this should
// be disallowed by the dialog too.

void FormFloat::build()
{
	dialog_.reset(build_float());

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->radio_top);
	bc().addReadOnly(dialog_->radio_bottom);
	bc().addReadOnly(dialog_->radio_page);
	bc().addReadOnly(dialog_->radio_here);
	bc().addReadOnly(dialog_->Here);
}


void FormFloat::apply()
{
	string placement;
	if (fl_get_button(dialog_->Here)) {
		placement += "H";
	} else {
		if (fl_get_button(dialog_->radio_top)) {
			placement += "t";
		}
		if (fl_get_button(dialog_->radio_bottom)) {
			placement += "b";
		}
		if (fl_get_button(dialog_->radio_page)) {
			placement += "p";
		}
		if (fl_get_button(dialog_->radio_here)) {
			placement += "h";
		}
	}
	controller().params().placement = placement;
}


void FormFloat::update()
{
	bool top = false;
	bool bottom = false;
	bool page = false;
	bool here = false;
	bool Here = false;

	string placement(controller().params().placement);
	
	if (contains(placement, "H")) {
		Here = true;
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
	fl_set_button(dialog_->radio_top, top);
	fl_set_button(dialog_->radio_bottom, bottom);
	fl_set_button(dialog_->radio_page, page);
	fl_set_button(dialog_->radio_here, here);
	fl_set_button(dialog_->Here, Here);

}

