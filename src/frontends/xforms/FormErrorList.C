/**
 * \file FormErrorList.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "errorlist.h"
#include "FormErrorList.h"
#include "xformsBC.h"
#include "xforms_helpers.h"
#include "ControlErrorList.h"
#include "forms/form_errorlist.h"
#include "support/lstrings.h" // frontStrip, strip
#include "debug.h"
#include "gettext.h"
#include "lyx_forms.h"


typedef FormController<ControlErrorList, FormView<FD_errorlist> > base_class;

FormErrorList::FormErrorList(Dialog & parent)
	: base_class(parent, "")
{}


int dumb_validator(FL_OBJECT *, const char *, const char *, int)
{
	return FL_INVALID;
}


void FormErrorList::build()
{
	dialog_.reset(build_errorlist(this));
	fl_set_input_filter(dialog_->input_description, dumb_validator);
}


void FormErrorList::update()
{
        setTitle(controller().name());
	updateContents();
}


ButtonPolicy::SMInput FormErrorList::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->browser_errors) {
		//xforms return values 1..n
		int const choice = int(fl_get_browser(dialog_->browser_errors)) - 1;
		goTo(choice);
	}

	return ButtonPolicy::SMI_VALID;
}


void FormErrorList::goTo(int where)
{
	ErrorList const & errors = controller().errorList();

	if (0 <= where && where < int(errors.size())) {
		controller().goTo(where);
		fl_set_input(dialog_->input_description,
			     errors[where].description.c_str());
		fl_set_input_topline(dialog_->input_description, 1);
	}
}


void FormErrorList::updateContents()
{
	fl_clear_browser(dialog_->browser_errors);

	ErrorList const & errors = controller().errorList();
	if (errors.empty()) {
		fl_add_browser_line(dialog_->browser_errors,
				    _("*** No Lists ***").c_str());
		setEnabled(dialog_->browser_errors, false);
		return;
	}

	setEnabled(dialog_->browser_errors, true);

	ErrorList::const_iterator cit = errors.begin();
	ErrorList::const_iterator end = errors.end();

	for (; cit != end; ++cit) {
		fl_add_browser_line(dialog_->browser_errors,
				    cit->error.c_str());
	}

	fl_select_browser_line(dialog_->browser_errors, 1);
	goTo(0);
}
