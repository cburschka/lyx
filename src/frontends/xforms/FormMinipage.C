/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormMinipage.C
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

typedef FormCB<ControlMinipage, FormDB<FD_form_minipage> > base_class;

FormMinipage::FormMinipage(ControlMinipage & c)
	: base_class(c, _("Minipage Options"))
{}


void FormMinipage::build()
{
	dialog_.reset(build_minipage());

	fl_set_input_return(dialog_->input_width, FL_RETURN_CHANGED);

	string const choice = getStringFromVector(minipage::getUnits(), "|");
	fl_addto_choice(dialog_->choice_width_units, subst(choice, "%", "%%").c_str());

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->input_width);
	bc().addReadOnly(dialog_->choice_width_units);
	bc().addReadOnly(dialog_->radio_top);
	bc().addReadOnly(dialog_->radio_middle);
	bc().addReadOnly(dialog_->radio_bottom);
}


void FormMinipage::apply()
{
	string const units = fl_get_choice_text(dialog_->choice_width_units);
	double const val = strToDbl(fl_get_input(dialog_->input_width));

	controller().params().width =
		tostr(val) + frontStrip(strip(subst(units,"%%","%")));

	if (fl_get_button(dialog_->radio_top))
		controller().params().pos = InsetMinipage::top;
	else if (fl_get_button(dialog_->radio_middle))
		controller().params().pos = InsetMinipage::center;
	else
		controller().params().pos = InsetMinipage::bottom;
}


void FormMinipage::update()
{
    LyXLength len(controller().params().width.c_str());
    fl_set_input(dialog_->input_width,tostr(len.value()).c_str());
    fl_set_choice(dialog_->choice_width_units, len.unit()+1);

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
