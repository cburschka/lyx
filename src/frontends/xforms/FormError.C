/*
 * \file FormError.C
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "xformsBC.h"
#include "ControlError.h"
#include "FormError.h"
#include "form_error.h"
#include "xforms_helpers.h" // formatted

typedef FormCB<ControlError, FormDB<FD_form_error> > base_class;

FormError::FormError(ControlError & c)
	: base_class(c, _("LaTeX Error"))
{}


void FormError::build()
{
	dialog_.reset(build_error());

        // Manage the cancel/close button
	bc().setCancel(dialog_->button_cancel);
}


void FormError::update()
{
	string const txt = formatted(controller().params(),
				     dialog_->message->w-10);
	fl_set_object_label(dialog_->message, txt.c_str());
}
