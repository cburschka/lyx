/**
 * \file FormThesaurus.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>
#include "support/lstrings.h"
#include "xformsBC.h"
#include "xforms_helpers.h"
#include "ControlThesaurus.h"
#include "FormThesaurus.h"
#include "form_thesaurus.h"
#include "debug.h"

typedef FormCB<ControlThesaurus, FormDB<FD_form_thesaurus> > base_class;

FormThesaurus::FormThesaurus(ControlThesaurus & c)
	: base_class(c, _("LyX: Thesaurus"), false),
	clickline_(-1)
{
}


void FormThesaurus::build()
{
	dialog_.reset(build_thesaurus());
 
	// Manage the ok, apply and cancel/close buttons
	bc().setCancel(dialog_->button_close);
	bc().addReadOnly(dialog_->input_replace);

	fl_set_input_return(dialog_->input_entry, FL_RETURN_END_CHANGED);
}


void FormThesaurus::update()
{
	if (!dialog_.get())
		return;

	string const & str_ = controller().text();
	setEnabled(dialog_->button_replace, !str_.empty());
	fl_set_input(dialog_->input_replace, "");
	updateMeanings(str_);
}


void FormThesaurus::updateMeanings(string const & str)
{
	fl_clear_browser(dialog_->browser_meanings);
 
	fl_set_input(dialog_->input_entry, str.c_str());

	fl_set_browser_topline(dialog_->browser_meanings, 1);

	fl_freeze_form(form());

	Thesaurus::Meanings meanings = controller().getMeanings(str);
 
	for (Thesaurus::Meanings::const_iterator cit = meanings.begin();
		cit != meanings.end(); ++cit) {
			fl_add_browser_line(dialog_->browser_meanings, cit->first.c_str());
			for (std::vector<string>::const_iterator cit2 = cit->second.begin();
				cit2 != cit->second.end(); ++cit2) {
					string ent = "   ";
					ent += *cit2;
					fl_add_browser_line(dialog_->browser_meanings, ent.c_str());
				}
		}
 
	fl_unfreeze_form(form());
	fl_redraw_form(form());
}


void FormThesaurus::setReplace(string const & templ, string const & nstr)
{
	string str(nstr);

	// the following mechanism makes sure we replace "House" with "Home",
	// "HOUSE" with "HOME" etc.

	bool all_lower = true;
	bool all_upper = true;

	for (string::const_iterator it = templ.begin(); it != templ.end(); ++it) {
		if (isupper(*it))
			all_lower = false;
		if (islower(*it))
			all_upper = false;
	}

	if (all_lower) {
		str = lowercase(nstr);
	} else if (all_upper) {
		str = uppercase(nstr);
	} else if (templ.size() > 0 && isupper(templ[0])) {
		bool rest_lower = true;
		for (string::const_iterator it = templ.begin() + 1;
			it != templ.end(); ++it) {
			if (isupper(*it))
				rest_lower = false;
		}
		
		if (rest_lower) {
			str = lowercase(nstr);
			str[0] = uppercase(nstr[0]);
		}
	}

	fl_set_input(dialog_->input_replace, str.c_str());
}


ButtonPolicy::SMInput FormThesaurus::input(FL_OBJECT * obj, long)
{
	if (obj == dialog_->input_entry) {
		string s = strip(frontStrip(fl_get_input(dialog_->input_entry)));
		 
		updateMeanings(s);

		if (s.empty()) {
			fl_set_input(dialog_->input_replace, "");
			return ButtonPolicy::SMI_APPLY;
		}
	} else if (obj == dialog_->button_replace) {
		string rep(fl_get_input(dialog_->input_replace));
		if (!rep.empty())
			controller().replace(fl_get_input(dialog_->input_replace));
		clickline_ = -1;
		updateMeanings(rep);
		return ButtonPolicy::SMI_APPLY;
	} else if (obj != dialog_->browser_meanings) {
		return ButtonPolicy::SMI_NOOP;
	}

	setReplace(fl_get_input(dialog_->input_entry),
		strip(frontStrip(fl_get_browser_line(obj, fl_get_browser(obj)))));

	if (clickline_ == fl_get_browser(obj)) {
		updateMeanings(fl_get_input(dialog_->input_replace));
		clickline_ = -1;
	} else {
		clickline_ = fl_get_browser(obj);
	}

	return ButtonPolicy::SMI_VALID;
}
