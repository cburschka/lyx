/**
 * \file FormPreamble.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlPreamble.h"
#include "FormPreamble.h"
#include "form_preamble.h"
#include "xforms_helpers.h"

typedef FormCB<ControlPreamble, FormDB<FD_form_preamble> > base_class;

FormPreamble::FormPreamble(ControlPreamble & c)
	: base_class(c, _("LaTeX preamble"))
{}


void FormPreamble::build()
{
	dialog_.reset(build_preamble());
   
	fl_set_input_return(dialog_->input_preamble, FL_RETURN_CHANGED);

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().addReadOnly(dialog_->input_preamble);
}


void FormPreamble::apply()
{
	controller().params() = fl_get_input(dialog_->input_preamble);
}


void FormPreamble::update()
{
	fl_set_input(dialog_->input_preamble, controller().params().c_str());

	bool const enable = (!controller().isReadonly());
	setEnabled(dialog_->input_preamble, enable);
	setEnabled(dialog_->button_ok,      enable);
	setEnabled(dialog_->button_apply,   enable);
}


