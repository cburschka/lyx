/**
 * \file FormFloat.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jürgen Spitzmüller
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlFloat.h"
#include "FormFloat.h"
#include "forms/form_float.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "support/lstrings.h"
#include FORMS_H_LOCATION

namespace {

enum {
	DOCUMENT_DEFAULTS,
	HERE_DEFINITELY,
	ALTERNATIVES
};
 
} // namespace anon


typedef FormCB<ControlFloat, FormDB<FD_float> > base_class;

FormFloat::FormFloat()
	: base_class(_("Float Options"))
{}


void FormFloat::build()
{
	dialog_.reset(build_float(this));

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	// disable for read-only documents
	bc().addReadOnly(dialog_->radio_default);
	bc().addReadOnly(dialog_->radio_here_definitely);
	bc().addReadOnly(dialog_->radio_alternatives);
	bc().addReadOnly(dialog_->check_top);
	bc().addReadOnly(dialog_->check_bottom);
	bc().addReadOnly(dialog_->check_page);
	bc().addReadOnly(dialog_->check_here);
	bc().addReadOnly(dialog_->check_force);
	bc().addReadOnly(dialog_->check_wide);

	placement_.init(dialog_->radio_default,         DOCUMENT_DEFAULTS);
	placement_.init(dialog_->radio_here_definitely, HERE_DEFINITELY);
	placement_.init(dialog_->radio_alternatives,    ALTERNATIVES);

	// set up the tooltips
	string str = _("Use the document's default settings.");
	tooltips().init(dialog_->radio_default, str);
	str = _("Enforce placement of float here.");
	tooltips().init(dialog_->radio_here_definitely, str);
	str = _("Alternative suggestions for placement of float.");
	tooltips().init(dialog_->radio_alternatives, str);
	str = _("Try top of page.");
	tooltips().init(dialog_->check_top, str);
	str = _("Try bottom of page.");
	tooltips().init(dialog_->check_bottom, str);
	str = _("Put float on a separate page of floats.");
	tooltips().init(dialog_->check_page, str);
	str = _("Try float here.");
	tooltips().init(dialog_->check_here, str);
	str = _("Ignore internal settings. This is the \"!\" in LaTeX.");
	tooltips().init(dialog_->check_force, str);
	str = _("Span float over the columns.");
	tooltips().init(dialog_->check_wide, str);
}


void FormFloat::apply()
{
	bool const wide = fl_get_button(dialog_->check_wide);

	string placement;
	switch (placement_.get()) {
	case ALTERNATIVES:
		if (fl_get_button(dialog_->check_force)) {
			// Ignore internal LaTeX rules
			placement += "!";
		}
		if (fl_get_button(dialog_->check_top)) {
			// Top of page
			placement += "t";
		}
		if (fl_get_button(dialog_->check_bottom)) {
			// Bottom of page
			placement += "b";
		}
		if (fl_get_button(dialog_->check_page)) {
			// Page of floats
			placement += "p";
		}
		// ignore if wide is selected
		if (!wide && fl_get_button(dialog_->check_here)) {
			// Here, if possible
			placement += "h";
		}
		if (placement == "!") {
			// ignore placement if only force is selected.
			placement.erase();
		}

		if (placement.length() == 0) {
			// none of Alternatives is selected; flip to default
			placement.erase();
			placement_.set(dialog_->radio_default);
			setEnabled(dialog_->check_force, false);
			setEnabled(dialog_->check_top, false);
			setEnabled(dialog_->check_bottom, false);
			setEnabled(dialog_->check_page, false);
			setEnabled(dialog_->check_here, false);
		}
		break;

	case HERE_DEFINITELY:
		placement = "H";
		break;

	case DOCUMENT_DEFAULTS:
		// default, do nothing.
		break;
	}

	controller().params().placement = placement;
	controller().params().wide = wide;
}


void FormFloat::update()
{
	string placement(controller().params().placement);
	bool const wide = controller().params().wide;

	bool const here_definitely = contains(placement, "H");

	bool const top    = contains(placement, "t");
	bool const bottom = contains(placement, "b");
	bool const page   = contains(placement, "p");
	bool const here   = contains(placement, "h");
	bool const force  = contains(placement, "!");
	bool const alternatives = top || bottom || page || (here && !wide);

	if (alternatives) {
		placement_.set(dialog_->radio_alternatives);
	} else if (here_definitely) {
		placement_.set(dialog_->radio_here_definitely);
	} else {
		placement_.set(dialog_->radio_default);
	}
	fl_set_button(dialog_->check_force, force);
	fl_set_button(dialog_->check_top, top);
	fl_set_button(dialog_->check_bottom, bottom);
	fl_set_button(dialog_->check_page, page);
	fl_set_button(dialog_->check_here, here);
	fl_set_button(dialog_->check_wide, wide);

	setEnabled(dialog_->radio_here_definitely, !wide);
	setEnabled(dialog_->check_force,  alternatives);
	setEnabled(dialog_->check_top,    alternatives);
	setEnabled(dialog_->check_bottom, alternatives);
	setEnabled(dialog_->check_page,   alternatives);
	setEnabled(dialog_->check_here,   alternatives && !wide);
}


ButtonPolicy::SMInput FormFloat::input(FL_OBJECT * ob, long)
{
	bool alternatives = placement_.get() == ALTERNATIVES;
	bool const wide = fl_get_button(dialog_->check_wide);

	if (ob == dialog_->radio_default ||
	    ob == dialog_->radio_here_definitely ||
	    ob == dialog_->radio_alternatives) {

		// set radio button
		placement_.set(ob);
		alternatives = placement_.get() == ALTERNATIVES;

		// enable check buttons for Alternatives
		setEnabled(dialog_->check_top, alternatives);
		setEnabled(dialog_->check_bottom, alternatives);
		setEnabled(dialog_->check_page, alternatives);
		// wide float doesn't allow 'here' placement
		setEnabled(dialog_->check_here, alternatives && !wide);

	} else if (ob == dialog_->check_wide) {
		// wide float doesn't allow 'Here, definitely!' and 'here'
		setEnabled(dialog_->radio_here_definitely, !wide);
		setEnabled(dialog_->check_here, alternatives && !wide);
		
		// flip to default, if 'Here, definitely!' was selected
		if (wide && placement_.get() == HERE_DEFINITELY) {
			placement_.set(dialog_->radio_default);
		}
	}

	// enable force button, if Alternatives is selected and at least
	// one of its check buttons
	bool const enable_force = alternatives &&
		(fl_get_button(dialog_->check_top) ||
		 fl_get_button(dialog_->check_bottom) ||
		 fl_get_button(dialog_->check_page) ||
		 (fl_get_button(dialog_->check_here) && !wide));
	setEnabled(dialog_->check_force, enable_force);

	return ButtonPolicy::SMI_VALID;
}
