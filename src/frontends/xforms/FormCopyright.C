// -*- C++ -*-
/*
 * \file FormCopyright.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Allan Rae, rae@lyx.org
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "xformsBC.h"
#include "ControlCopyright.h"
#include "FormCopyright.h"
#include "form_copyright.h"
#include "xforms_helpers.h"

typedef FormCB<ControlCopyright, FormDB<FD_form_copyright> > base_class;

FormCopyright::FormCopyright(ControlCopyright & c)
	: base_class(c, _("Copyright and Warranty"))
{}


void FormCopyright::build()
{
	dialog_.reset(build_copyright());

	string str = formatted(controller().getCopyright(),
			       dialog_->text_copyright->w-10);

	fl_set_object_label(dialog_->text_copyright, str.c_str());

	str = formatted(controller().getLicence(),
			dialog_->text_licence->w-10);

	fl_set_object_label(dialog_->text_licence, str.c_str());

	str = formatted(controller().getDisclaimer(),
			dialog_->text_disclaimer->w-10);

	fl_set_object_label(dialog_->text_disclaimer, str.c_str());
	
        // Manage the cancel/close button
	bc().setCancel(dialog_->button_cancel);
}
