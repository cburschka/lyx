/**
 * \file FormMinipage.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "xformsBC.h"
#include "ControlMinipage.h"
#include "FormMinipage.h"
#include "forms/form_minipage.h"
#include "support/lstrings.h"
#include "support/tostr.h"
#include "helper_funcs.h"
#include "debug.h"
#include "xforms_helpers.h"
#include "lyx_forms.h"

typedef FormController<ControlMinipage, FormView<FD_minipage> > base_class;

FormMinipage::FormMinipage(Dialog & parent)
	: base_class(parent, _("Minipage Settings"))
{}


void FormMinipage::build()
{
	dialog_.reset(build_minipage(this));

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	fl_set_input_return(dialog_->input_width, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input_width);

	string const choice = getStringFromVector(getLatexUnits(), "|");
	fl_addto_choice(dialog_->choice_width_units, subst(choice, "%", "%%").c_str());

	// Manage the ok, apply and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().setRestore(dialog_->button_restore);

	bcview().addReadOnly(dialog_->input_width);
	bcview().addReadOnly(dialog_->choice_width_units);
	bcview().addReadOnly(dialog_->radio_top);
	bcview().addReadOnly(dialog_->radio_middle);
	bcview().addReadOnly(dialog_->radio_bottom);
}


void FormMinipage::apply()
{
	controller().params().width =
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
	LyXLength len(controller().params().width);
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
	clearMessage();

	ButtonPolicy::SMInput action = ButtonPolicy::SMI_NOOP;

	if (ob == dialog_->radio_top ||
	    ob == dialog_->radio_middle ||
	    ob == dialog_->radio_bottom ||
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
