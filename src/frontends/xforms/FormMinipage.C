/**
 * \file FormMinipage.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Jürgen Vigna, jug@sad.it
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlMinipage.h"
#include "FormMinipage.h"
#include "form_minipage.h"
#include "support/lstrings.h"
#include "helper_funcs.h"
#include "debug.h"
#include "xforms_helpers.h"

typedef FormCB<ControlMinipage, FormDB<FD_form_minipage> > base_class;

FormMinipage::FormMinipage(ControlMinipage & c)
	: base_class(c, _("Minipage Options"))
{}


void FormMinipage::build()
{
	dialog_.reset(build_minipage());

	fl_set_input_return(dialog_->input_width, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input_width);

	string const choice = getStringFromVector(getLatexUnits(), "|");
	fl_addto_choice(dialog_->choice_width_units, subst(choice, "%", "%%").c_str());

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->input_width);
	bc().addReadOnly(dialog_->choice_width_units);
	bc().addReadOnly(dialog_->group_alignment);
	bc().addReadOnly(dialog_->radio_top);
	bc().addReadOnly(dialog_->radio_middle);
	bc().addReadOnly(dialog_->radio_bottom);
}


void FormMinipage::apply()
{
	controller().params().pageWidth =
		LyXLength(getLengthFromWidgets(dialog_->input_width,
			dialog_->choice_width_units));

	if (fl_get_button(dialog_->radio_top))
		controller().params().pos = InsetMinipage::top;
	else if (fl_get_button(dialog_->radio_middle))
		controller().params().pos = InsetMinipage::center;
	else
		controller().params().pos = InsetMinipage::bottom;
}


void FormMinipage::update()
{
    LyXLength len(controller().params().pageWidth);
    fl_set_input(dialog_->input_width, tostr(len.value()).c_str());
    fl_set_choice(dialog_->choice_width_units, len.unit() + 1);

    switch (controller().params().pos) {
    case InsetMinipage::top:
	fl_set_button(dialog_->radio_top, 1);
	break;
    case InsetMinipage::center:
	fl_set_button(dialog_->radio_middle, 1);
	break;
    case InsetMinipage::bottom:
	fl_set_button(dialog_->radio_bottom, 1);
	break;
    }
}

ButtonPolicy::SMInput FormMinipage::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput action = ButtonPolicy::SMI_NOOP;

	if (ob == dialog_->radio_top || 
		ob == dialog_->radio_middle ||
		ob == dialog_->radio_bottom ||
		ob == dialog_->choice_width_units)
		return ButtonPolicy::SMI_VALID;

	// disallow senseless data
	// warnings if input is senseless
	if (ob == dialog_->input_width) {
		string const input = getStringFromInput(dialog_->input_width);
		bool const invalid = !isValidLength(input) && !isStrDbl(input);
		if (invalid) {
			fl_set_object_label(dialog_->text_warning,
				_("Warning: Invalid Length!"));
			fl_show_object(dialog_->text_warning);
			action = ButtonPolicy::SMI_INVALID;
		} else {
			fl_hide_object(dialog_->text_warning);
			action = ButtonPolicy::SMI_VALID;
		}
	}

	return action;
}
