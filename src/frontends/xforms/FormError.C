/*
 * \file FormError.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "xformsBC.h"
#include "ControlError.h"
#include "FormError.h"
#include "forms/form_error.h"
#include "xforms_helpers.h" // formatted
#include FORMS_H_LOCATION

typedef FormCB<ControlError, FormDB<FD_error> > base_class;

FormError::FormError(ControlError & c)
	: base_class(c, _("LaTeX Error"))
{}


void FormError::build()
{
	dialog_.reset(build_error(this));

	// Manage the cancel/close button
	bc().setCancel(dialog_->button_close);
}


void FormError::update()
{
	string const txt = formatted(controller().params(),
				     dialog_->frame_message->w - 10);
	fl_set_object_label(dialog_->frame_message, txt.c_str());
}
