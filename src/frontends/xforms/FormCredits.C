/**
 * \file FormCredits.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "xformsBC.h"
#include "ControlCredits.h"
#include "FormCredits.h"
#include "form_credits.h"
#include "xforms_helpers.h"
#include "Lsstream.h"

using std::getline;

typedef FormCB<ControlCredits, FormDB<FD_form_credits> > base_class;

FormCredits::FormCredits(ControlCredits & c)
	: base_class(c, _("Credits"))
{}


void FormCredits::build()
{
	dialog_.reset(build_credits());

	// Manage the cancel/close button
	bc().setCancel(dialog_->button_cancel);

	std::stringstream ss;
	fl_add_browser_line(dialog_->browser_credits, 
			    controller().getCredits(ss).str().c_str());
}
