/**
 * \file FormThesaurus.C
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

#include "ControlThesaurus.h"
#include "FormThesaurus.h"
#include "forms/form_thesaurus.h"
#include "xformsBC.h"
#include "xforms_helpers.h"
#include "debug.h"
#include "support/lstrings.h"
#include FORMS_H_LOCATION
#include <cctype>

#ifndef CXX_GLOBAL_CSTD
using std::isupper;
using std::islower;
#endif

using std::vector;


typedef FormCB<ControlThesaurus, FormDB<FD_thesaurus> > base_class;


FormThesaurus::FormThesaurus()
	: base_class(_("LyX: Thesaurus"), false),
	  clickline_(-1)
{
}


void FormThesaurus::build()
{
	dialog_.reset(build_thesaurus(this));

	fl_set_input_return(dialog_->input_entry,   FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_replace, FL_RETURN_CHANGED);

	setPrehandler(dialog_->input_entry);
	setPrehandler(dialog_->input_replace);

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

	Thesaurus::Meanings::const_iterator cit = meanings.begin();
	Thesaurus::Meanings::const_iterator end = meanings.end();
	for (; cit != end; ++cit) {
		fl_add_browser_line(dialog_->browser_meanings,
				    cit->first.c_str());

		vector<string> const & tmpvec = cit->second;
		vector<string>::const_iterator cit2 = tmpvec.begin();
		vector<string>::const_iterator end2 = tmpvec.end();
		for (; cit2 != end2; ++cit2) {
			string ent = "   ";
			ent += *cit2;
			fl_add_browser_line(dialog_->browser_meanings,
					    ent.c_str());
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

	string::const_iterator beg = templ.begin();
	string::const_iterator end = templ.end();
	string::const_iterator cit = beg;
	for (; cit != end; ++cit) {
		if (isupper(*cit))
			all_lower = false;
		if (islower(*cit))
			all_upper = false;
	}

	if (all_lower) {
		str = lowercase(nstr);
	} else if (all_upper) {
		str = uppercase(nstr);
	} else if (templ.size() > 0 && isupper(templ[0])) {
		bool rest_lower = true;
		string::const_iterator cit2 = beg + 1;

		for (; cit2 != end; ++cit2) {
			if (isupper(*cit2))
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
		string s = trim(fl_get_input(dialog_->input_entry));

		updateMeanings(s);

		if (s.empty()) {
			fl_set_input(dialog_->input_replace, "");
			return ButtonPolicy::SMI_APPLY;
		}
		return ButtonPolicy::SMI_NOOP;

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

	int const line = fl_get_browser(obj);
	if (line > 0) {
		setReplace(fl_get_input(dialog_->input_entry),
			   trim(fl_get_browser_line(obj, line)));
	}

	if (clickline_ == fl_get_browser(obj)) {
		updateMeanings(fl_get_input(dialog_->input_replace));
		clickline_ = -1;
	} else {
		clickline_ = fl_get_browser(obj);
	}

	return ButtonPolicy::SMI_VALID;
}
