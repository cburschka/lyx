/**
 * \file FormWrap.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlWrap.h"
#include "FormWrap.h"
#include "forms/form_wrap.h"
#include "support/lstrings.h"
#include "helper_funcs.h"
#include "debug.h"
#include "xforms_helpers.h"
#include FORMS_H_LOCATION

typedef FormCB<ControlWrap, FormDB<FD_wrap> > base_class;

FormWrap::FormWrap()
	: base_class(_("Wrap Options"))
{}


void FormWrap::build()
{
	dialog_.reset(build_wrap(this));

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	fl_set_input_return(dialog_->input_width, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input_width);

	string const choice = getStringFromVector(getLatexUnits(), "|");
	fl_addto_choice(dialog_->choice_width_units, subst(choice, "%", "%%").c_str());

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->input_width);
	bc().addReadOnly(dialog_->choice_width_units);
	bc().addReadOnly(dialog_->radio_left);
	bc().addReadOnly(dialog_->radio_right);
	bc().addReadOnly(dialog_->radio_inner);
	bc().addReadOnly(dialog_->radio_default);
}


void FormWrap::apply()
{
	controller().params().pageWidth =
		LyXLength(getLengthFromWidgets(dialog_->input_width,
					       dialog_->choice_width_units));

	if (fl_get_button(dialog_->radio_left))
		controller().params().placement = "l";
	else if (fl_get_button(dialog_->radio_right))
		controller().params().placement = "r";
	else if (fl_get_button(dialog_->radio_inner))
		controller().params().placement = "p";
	else
		controller().params().placement.erase();
}


void FormWrap::update()
{
	LyXLength len(controller().params().pageWidth);
	fl_set_input(dialog_->input_width, tostr(len.value()).c_str());
	fl_set_choice(dialog_->choice_width_units, len.unit() + 1);

	if (controller().params().placement == "l")
		fl_set_button(dialog_->radio_left, 1);
	else if (controller().params().placement == "r")
		fl_set_button(dialog_->radio_right, 1);
	else if (controller().params().placement == "p")
		fl_set_button(dialog_->radio_inner, 1);
	else
		fl_set_button(dialog_->radio_default, 1);
}


ButtonPolicy::SMInput FormWrap::input(FL_OBJECT * ob, long)
{
	clearMessage();

	ButtonPolicy::SMInput action = ButtonPolicy::SMI_NOOP;

	if (ob == dialog_->radio_left ||
	    ob == dialog_->radio_right ||
	    ob == dialog_->radio_inner ||
	    ob == dialog_->radio_default ||
	    ob == dialog_->choice_width_units)
		return ButtonPolicy::SMI_VALID;

	// disallow senseless data
	// warnings if input is senseless
	if (ob == dialog_->input_width) {
		string const input = getString(dialog_->input_width);
		bool const invalid = !isValidLength(input) && !isStrDbl(input);
		if (invalid) {
			postWarning(_("Invalid Length!"));
			action = ButtonPolicy::SMI_INVALID;
		} else {
			action = ButtonPolicy::SMI_VALID;
		}
	}

	return action;
}
