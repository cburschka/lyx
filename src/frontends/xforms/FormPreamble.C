/**
 * \file FormPreamble.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlPreamble.h"
#include "FormPreamble.h"
#include "forms/form_preamble.h"
#include "xforms_helpers.h"
#include FORMS_H_LOCATION

typedef FormCB<ControlPreamble, FormDB<FD_preamble> > base_class;

FormPreamble::FormPreamble()
	: base_class(_("LaTeX preamble"))
{}


void FormPreamble::build()
{
	dialog_.reset(build_preamble(this));

	fl_set_input_return(dialog_->input_preamble, FL_RETURN_CHANGED);

	// trigger an input event when pasting using the middle mouse button.
	setPrehandler(dialog_->input_preamble);

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
}


void FormPreamble::apply()
{
	controller().params() = fl_get_input(dialog_->input_preamble);
}


void FormPreamble::update()
{
	fl_set_input(dialog_->input_preamble, controller().params().c_str());
}
