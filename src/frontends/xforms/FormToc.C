/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormToc.C
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>
#include <vector>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlToc.h"
#include "FormToc.h"
#include "form_toc.h"
#include "helper_funcs.h" // getStringFromVector
#include "support/lstrings.h" // frontStrip, strip


typedef FormCB<ControlToc, FormDB<FD_form_toc> > base_class;

FormToc::FormToc(ControlToc & c)
	: base_class(c, _("Table of Contents"))
{}


void FormToc::build()
{
	dialog_.reset(build_toc());

	string const choice =
		" " + getStringFromVector(controller().getTypes(), " | ") + " ";
	fl_addto_choice(dialog_->choice_toc_type, choice.c_str());

        // Manage the cancel/close button
	bc().setCancel(dialog_->button_cancel);
	bc().refresh();
}


void FormToc::update()
{
	updateType();
	updateContents();
}


ButtonPolicy::SMInput FormToc::input(FL_OBJECT *, long)
{
	updateContents();

	unsigned int const choice = fl_get_browser( dialog_->browser_toc );
	if (0 < choice && choice - 1 < toclist_.size()) {
		controller().Goto(toclist_[choice-1].par->id());
	}

	return ButtonPolicy::SMI_VALID;
}


void FormToc::updateType()
{
	string const type = toc::getType(controller().params().getCmdName());
	
	fl_set_choice(dialog_->choice_toc_type, 1);
	for (int i = 1;
	     i <= fl_get_choice_maxitems(dialog_->choice_toc_type); ++i) {
		string const choice =
			fl_get_choice_item_text(dialog_->choice_toc_type, i);

		if (choice == type) {
			fl_set_choice(dialog_->choice_toc_type, i);
			break;
		}
	}
}


void FormToc::updateContents()
{
	string const type =
		frontStrip(strip(fl_get_choice_text(dialog_->choice_toc_type)));

	Buffer::SingleList const contents = controller().getContents(type);

	// Check if all elements are the same.
	if (toclist_ == contents) {
		return;
	}
	
	// List has changed. Update browser
	toclist_ = contents;

	unsigned int const topline =
		fl_get_browser_topline(dialog_->browser_toc);
	unsigned int const line = fl_get_browser(dialog_->browser_toc);

	fl_clear_browser( dialog_->browser_toc );

	Buffer::SingleList::const_iterator cit = toclist_.begin();
	Buffer::SingleList::const_iterator end = toclist_.end();
	
	for (; cit != end; ++cit) {
		string const line = string(4 * cit->depth, ' ') + cit->str;
		fl_add_browser_line(dialog_->browser_toc, line.c_str());
	}
	
	fl_set_browser_topline(dialog_->browser_toc, topline);
	fl_select_browser_line(dialog_->browser_toc, line);
}
