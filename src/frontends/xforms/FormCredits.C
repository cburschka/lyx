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

using std::vector;

typedef FormCB<ControlCredits, FormDB<FD_form_credits> > base_class;

FormCredits::FormCredits(ControlCredits & c)
	: base_class(c, _("Credits"))
{}


void FormCredits::build()
{
	dialog_.reset(build_credits());

	// Manage the cancel/close button
	bc().setCancel(dialog_->button_cancel);
	bc().refresh();

	vector<string> data = controller().getCredits();

	/* read the credits into the browser */ 
	for (vector<string>::const_iterator it = data.begin();
	     it < data.end(); ++it) {
		fl_add_browser_line(dialog_->browser_credits, it->c_str());
	}
}
