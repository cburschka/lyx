/**
 * \file FormSearch.C
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
#include "ControlSearch.h"
#include "FormSearch.h"
#include "forms/form_search.h"
#include "Tooltips.h"
#include "xforms_helpers.h"

#include FORMS_H_LOCATION

typedef FormCB<ControlSearch, FormDB<FD_search> > base_class;

FormSearch::FormSearch()
	: base_class(_("Find and Replace"))
{}


void FormSearch::build()
{
	dialog_.reset(build_search(this));

	// Manage the ok, apply and cancel/close buttons
	bc().setCancel(dialog_->button_close);

	// disable for read-only documents
	bc().addReadOnly(dialog_->input_replace);
	bc().addReadOnly(dialog_->button_replace);
	bc().addReadOnly(dialog_->button_replaceall);

	// set up the tooltips
	string str = _("Enter the string you want to find.");
	tooltips().init(dialog_->input_search, str);
	str = _("Enter the replacement string.");
	tooltips().init(dialog_->input_replace, str);
	str = _("Continue to next search result.");
	tooltips().init(dialog_->button_findnext, str);
	str = _("Replace search result by replacement string.");
	tooltips().init(dialog_->button_replace, str);
	str = _("Replace all by replacement string.");
	tooltips().init(dialog_->button_replaceall, str);
	str = _("Do case sensitive search.");
	tooltips().init(dialog_->check_casesensitive, str);
	str = _("Search only matching words.");
	tooltips().init(dialog_->check_matchword, str);
	str = _("Search backwards.");
	tooltips().init(dialog_->check_searchbackwards, str);
}


void FormSearch::update()
{
	fl_set_input_selected(dialog_->input_search, true);
	fl_set_focus_object(dialog_->form, dialog_->input_search);
}


ButtonPolicy::SMInput FormSearch::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_findnext) {
		controller().find(getString(dialog_->input_search),
				  fl_get_button(dialog_->check_casesensitive),
				  fl_get_button(dialog_->check_matchword),
				  !fl_get_button(dialog_->check_searchbackwards));

	} else if (ob == dialog_->button_replace || ob == dialog_->button_replaceall) {
		bool const all = (ob == dialog_->button_replaceall);

		controller().replace(getString(dialog_->input_search),
				     getString(dialog_->input_replace),
				     fl_get_button(dialog_->check_casesensitive),
				     fl_get_button(dialog_->check_matchword),
				     all);
	}

	return ButtonPolicy::SMI_VALID;
}
