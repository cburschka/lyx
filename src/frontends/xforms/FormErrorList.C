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


#include "FormErrorList.h"
#include "xformsBC.h"
#include "xforms_helpers.h"
#include "ControlErrorList.h"
#include "forms/form_errorlist.h"
#include "support/lstrings.h" // frontStrip, strip
#include "debug.h"
#include "gettext.h"
#include "lyx_forms.h"

#include <vector>

using std::vector;
using std::endl;


typedef FormController<ControlErrorList, FormView<FD_errorlist> > base_class;

FormErrorList::FormErrorList(Dialog & parent)
	: base_class(parent, _("LaTeX error list"))
{}


void FormErrorList::build()
{
	dialog_.reset(build_errorlist(this));

	// Manage the cancel/close button
	bcview().setCancel(dialog_->button_close);
	bcview().addReadOnly(dialog_->browser_errors);
}


void FormErrorList::update()
{
	updateContents();
}


ButtonPolicy::SMInput FormErrorList::input(FL_OBJECT * ob, long)
{
	std::vector<ControlErrorList::ErrorItem> const &
		Errors = controller().ErrorList();

	if (ob == dialog_->browser_errors) {
		//xforms return values 1..n
		int const choice = int(fl_get_browser(dialog_->browser_errors)) - 1;
		if (0 <= choice && choice < int(Errors.size())) {
			controller().goTo(choice);
			fl_set_input(dialog_->input_description,
				     Errors[choice].description.c_str());
		}
		return ButtonPolicy::SMI_VALID;
	}

	updateContents();

	return ButtonPolicy::SMI_VALID;
}


void FormErrorList::updateContents()
{
	std::vector<ControlErrorList::ErrorItem> const &
		Errors = controller().ErrorList();

	if (Errors.empty()) {
		fl_clear_browser(dialog_->browser_errors);
		fl_add_browser_line(dialog_->browser_errors,
				    _("*** No Lists ***").c_str());
		setEnabled(dialog_->browser_errors, false);
		return;
	}

	unsigned int const topline =
		fl_get_browser_topline(dialog_->browser_errors);
	unsigned int const line = fl_get_browser(dialog_->browser_errors);

	fl_clear_browser(dialog_->browser_errors);
	setEnabled(dialog_->browser_errors, true);

	std::vector<ControlErrorList::ErrorItem>::const_iterator
		cit = Errors.begin();
	std::vector<ControlErrorList::ErrorItem>::const_iterator
		end = Errors.end();

	for (; cit != end; ++cit) {
		fl_add_browser_line(dialog_->browser_errors,
				    cit->error.c_str());
	}

	fl_set_browser_topline(dialog_->browser_errors, topline);
	fl_select_browser_line(dialog_->browser_errors, line);
}
