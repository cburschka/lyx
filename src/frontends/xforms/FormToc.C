/**
 * \file FormToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormToc.h"
#include "ControlToc.h"
#include "forms/form_toc.h"

#include "xforms_helpers.h"
#include "xformsBC.h"

#include "support/lstrings.h" // frontStrip, strip

#include "lyx_forms.h"

using namespace lyx::support;

using std::vector;


typedef FormController<ControlToc, FormView<FD_toc> > base_class;

FormToc::FormToc(Dialog & parent)
	: base_class(parent, _("Table of Contents"))
{}


void FormToc::build()
{
	dialog_.reset(build_toc(this));

	vector<string> types = controller().getTypes();


	string const choice =
		' ' + getStringFromVector(controller().getTypes(), " | ") + ' ';
	fl_addto_choice(dialog_->choice_toc_type, choice.c_str());

	// Manage the cancel/close button
	bcview().setCancel(dialog_->button_close);
}


void FormToc::update()
{
	updateType();
	updateContents();
}


ButtonPolicy::SMInput FormToc::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->browser_toc) {
		unsigned int const choice = fl_get_browser(dialog_->browser_toc);

		if (choice - 1 < toc_.size() && choice >= 1) {
			controller().goTo(toc_[choice - 1]);
		}
		return ButtonPolicy::SMI_VALID;
	}

	if (ob != dialog_->choice_toc_type) {
		updateType();
	}

	updateContents();

	return ButtonPolicy::SMI_VALID;
}


void FormToc::updateType()
{
	// Update the choice list from scratch
	fl_clear_choice(dialog_->choice_toc_type);
	string const choice = getStringFromVector(controller().getTypes(), "|");
	fl_addto_choice(dialog_->choice_toc_type, choice.c_str());

	// And select the correct one
	string const type = lyx::toc::getType(controller().params().getCmdName());
	fl_set_choice_text(dialog_->choice_toc_type, type.c_str());
}


void FormToc::updateContents()
{
	string const type = getString(dialog_->choice_toc_type);
	if (type.empty()) {
		fl_clear_browser(dialog_->browser_toc);
		fl_add_browser_line(dialog_->browser_toc,
				    _("*** No Lists ***").c_str());
		setEnabled(dialog_->browser_toc, false);
		return;
	}

	lyx::toc::Toc const contents = controller().getContents(type);

	// Check if all elements are the same.
	if (toc_ == contents) {
		return;
	}

	// List has changed. Update browser
	toc_ = contents;

	if (contents.empty()) {
		fl_clear_browser(dialog_->browser_toc);
		fl_add_browser_line(dialog_->browser_toc,
				    _("*** No Lists ***").c_str());
		setEnabled(dialog_->browser_toc, false);
		return;
	}

	unsigned int const topline =
		fl_get_browser_topline(dialog_->browser_toc);
	unsigned int const line = fl_get_browser(dialog_->browser_toc);

	fl_clear_browser(dialog_->browser_toc);
	setEnabled(dialog_->browser_toc, true);

	lyx::toc::Toc::const_iterator cit = contents.begin();
	lyx::toc::Toc::const_iterator end = contents.end();
	for (; cit != end; ++cit) {
		fl_add_browser_line(dialog_->browser_toc,
				    cit->asString().c_str());
	}

	fl_set_browser_topline(dialog_->browser_toc, topline);
	fl_select_browser_line(dialog_->browser_toc, line);
}
