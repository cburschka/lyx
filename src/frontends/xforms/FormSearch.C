// -*- C++ -*-
/**
 * \file FormSearch.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlSearch.h"
#include "FormSearch.h"
#include "form_search.h"

typedef FormCB<ControlSearch, FormDB<FD_form_search> > base_class;

FormSearch::FormSearch(ControlSearch & c)
	: base_class(c, _("LyX: Find and Replace"))
{}


void FormSearch::build()
{
	dialog_.reset(build_search());
	
	// Manage the ok, apply and cancel/close buttons
	bc().setCancel(dialog_->button_cancel);
	bc().addReadOnly(dialog_->input_replace);
	bc().addReadOnly(dialog_->replace);
	bc().addReadOnly(dialog_->replaceall);
}


ButtonPolicy::SMInput FormSearch::input(FL_OBJECT * obj, long)
{
	if (obj == dialog_->findnext || obj == dialog_->findprev) {
		bool const forward = (obj == dialog_->findnext);
	
		controller().find(fl_get_input(dialog_->input_search),
				  fl_get_button(dialog_->casesensitive),
				  fl_get_button(dialog_->matchword),
				  forward);

	} else if (obj == dialog_->replace || obj == dialog_->replaceall) {
		bool const all = (obj == dialog_->replaceall);
	
		controller().replace(fl_get_input(dialog_->input_search),
				     fl_get_input(dialog_->input_replace),
				     fl_get_button(dialog_->casesensitive),
				     fl_get_button(dialog_->matchword),
				     all);
	}
   
	return ButtonPolicy::SMI_VALID;
}
