/**
 * \file FormERT.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "xformsBC.h"
#include "ControlERT.h"
#include "FormERT.h"
#include "forms/form_ert.h"
#include "support/lstrings.h"
#include "helper_funcs.h"
#include "debug.h"
#include "lyx_forms.h"

typedef  FormController<ControlERT, FormView<FD_ert> > base_class;

FormERT::FormERT(Dialog & parent)
	: base_class(parent, _("TeX Settings"))
{}


void FormERT::build()
{
	dialog_.reset(build_ert(this));

	// Manage the ok, apply and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);

	bcview().addReadOnly(dialog_->radio_open);
	bcview().addReadOnly(dialog_->radio_collapsed);
	bcview().addReadOnly(dialog_->radio_inlined);
}


void FormERT::apply()
{
	if (fl_get_button(dialog_->radio_open))
		controller().setStatus(InsetERT::Open);
	else if (fl_get_button(dialog_->radio_collapsed))
		controller().setStatus(InsetERT::Collapsed);
	else
		controller().setStatus(InsetERT::Inlined);
}


void FormERT::update()
{
	switch (controller().status()) {
	case InsetERT::Open:
		fl_set_button(dialog_->radio_open, 1);
		break;
	case InsetERT::Collapsed:
		fl_set_button(dialog_->radio_collapsed, 1);
		break;
	case InsetERT::Inlined:
		fl_set_button(dialog_->radio_inlined, 1);
		break;
	}
}
