/**
 * \file xforms/FormToc.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FormToc.h"
#include "xformsBC.h"
#include "xforms_helpers.h"
#include "ControlToc.h"
#include "forms/form_toc.h"
#include "support/lstrings.h" // frontStrip, strip
#include "debug.h"
#include "gettext.h"
#include FORMS_H_LOCATION

#include <vector>

using std::vector;
using std::endl;


typedef FormCB<ControlToc, FormDB<FD_toc> > base_class;

FormToc::FormToc(ControlToc & c, Dialogs & d)
	: base_class(c, d, _("Table of Contents"))
{}


void FormToc::build()
{
	dialog_.reset(build_toc(this));

	vector<string> types = controller().getTypes();
	
	
	string const choice =
		" " + getStringFromVector(controller().getTypes(), " | ") + " ";
	fl_addto_choice(dialog_->choice_toc_type, choice.c_str());

	// Manage the cancel/close button
	bc().setCancel(dialog_->button_close);
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
	string const type = toc::getType(controller().params().getCmdName());
	fl_set_choice_text(dialog_->choice_toc_type, type.c_str());
}


void FormToc::updateContents()
{
	string const type = getString(dialog_->choice_toc_type);
	if (type.empty()) {
		fl_clear_browser(dialog_->browser_toc);
		fl_add_browser_line(dialog_->browser_toc,
				    _("*** No Lists ***"));
		setEnabled(dialog_->browser_toc, false);
		return;
	}

	toc::Toc const contents = controller().getContents(type);

	// Check if all elements are the same.
	if (toc_ == contents) {
		return;
	}

	// List has changed. Update browser
	toc_ = contents;

	if (contents.empty()) {
		fl_clear_browser(dialog_->browser_toc);
		fl_add_browser_line(dialog_->browser_toc,
				    _("*** No Lists ***"));
		setEnabled(dialog_->browser_toc, false);
		return;
	}

	unsigned int const topline =
		fl_get_browser_topline(dialog_->browser_toc);
	unsigned int const line = fl_get_browser(dialog_->browser_toc);

	fl_clear_browser(dialog_->browser_toc);
	setEnabled(dialog_->browser_toc, true);

	toc::Toc::const_iterator cit = contents.begin();
	toc::Toc::const_iterator end = contents.end();
	for (; cit != end; ++cit) {
		fl_add_browser_line(dialog_->browser_toc,
				    cit->asString().c_str());
	}

	fl_set_browser_topline(dialog_->browser_toc, topline);
	fl_select_browser_line(dialog_->browser_toc, line);
}
