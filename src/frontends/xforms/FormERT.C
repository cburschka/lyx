/**
 * \file xforms/FormERT.C
 * See the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlERT.h"
#include "FormERT.h"
#include "forms/form_ert.h"
#include "support/lstrings.h"
#include "helper_funcs.h"
#include "debug.h"
#include FORMS_H_LOCATION

typedef FormCB<ControlERT, FormDB<FD_ert> > base_class;

FormERT::FormERT()
	: base_class(_("ERT Options"))
{}


void FormERT::build()
{
	dialog_.reset(build_ert(this));

	// Manage the ok, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->radio_open);
	bc().addReadOnly(dialog_->radio_collapsed);
	bc().addReadOnly(dialog_->radio_inlined);
}


void FormERT::apply()
{
	if (fl_get_button(dialog_->radio_open))
		controller().params().status = InsetERT::Open;
	else if (fl_get_button(dialog_->radio_collapsed))
		controller().params().status = InsetERT::Collapsed;
	else
		controller().params().status = InsetERT::Inlined;
}


void FormERT::update()
{
	switch (controller().params().status) {
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
