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

typedef FormCB<ControlThesaurus, FormDB<FD_form_tabbed_thesaurus> > base_class;

FormThesaurus::FormThesaurus(ControlThesaurus & c)
	: base_class(c, _("LyX: Thesaurus")),
	clickline_(-1)
{
}


void FormThesaurus::build()
{
	dialog_.reset(build_tabbed_thesaurus());
	noun_.reset(build_noun());
	verb_.reset(build_verb());
	adjective_.reset(build_adjective());
	adverb_.reset(build_adverb());
	other_.reset(build_other());

	fl_set_form_maxsize(dialog_->form, minw_, minh_);

	// Manage the ok, apply and cancel/close buttons
	bc().setCancel(dialog_->button_close);
	bc().addReadOnly(dialog_->input_replace);

	fl_set_input_return(dialog_->input_entry, FL_RETURN_END_CHANGED);

	fl_addto_tabfolder(dialog_->tabbed_folder, _("Nouns"), noun_->form);
	fl_addto_tabfolder(dialog_->tabbed_folder, _("Verbs"), verb_->form);
	fl_addto_tabfolder(dialog_->tabbed_folder, _("Adjectives"), adjective_->form);
	fl_addto_tabfolder(dialog_->tabbed_folder, _("Adverbs"), adverb_->form);
	fl_addto_tabfolder(dialog_->tabbed_folder, _("Other"), other_->form);
}


void FormThesaurus::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
	else
		return;

	FL_FORM * form = fl_get_active_folder(dialog_->tabbed_folder);
	if (form && form->visible)
		fl_redraw_form(form);
}


void FormThesaurus::update()
{
	if (!dialog_.get())
		return;

	string const & str_ = controller().text();
	setEnabled(dialog_->button_replace, !str_.empty());
	fl_set_input(dialog_->input_replace, "");
	updateEntries(str_);
}


void FormThesaurus::updateEntries(string const & str)
{
	fl_set_input(dialog_->input_entry, str.c_str());

	fl_clear_browser(noun_->browser_noun);
	fl_clear_browser(verb_->browser_verb);
	fl_clear_browser(adjective_->browser_adjective);
	fl_clear_browser(adverb_->browser_adverb);
	fl_clear_browser(other_->browser_other);

	fl_set_browser_topline(noun_->browser_noun, 1);
	fl_set_browser_topline(verb_->browser_verb, 1);
	fl_set_browser_topline(adjective_->browser_adjective, 1);
	fl_set_browser_topline(adverb_->browser_adverb, 1);
	fl_set_browser_topline(other_->browser_other, 1);

	fl_freeze_form(noun_->form);
	fl_freeze_form(verb_->form);
	fl_freeze_form(adverb_->form);
	fl_freeze_form(adjective_->form);
	fl_freeze_form(other_->form);

	std::vector<string> nouns = controller().getNouns(str);
	for (std::vector<string>::const_iterator it = nouns.begin(); it != nouns.end(); ++it)
		fl_add_browser_line(noun_->browser_noun, it->c_str());

	std::vector<string> verbs = controller().getVerbs(str);
	for (std::vector<string>::const_iterator it = verbs.begin(); it != verbs.end(); ++it)
		fl_add_browser_line(verb_->browser_verb, it->c_str());

	std::vector<string> adjectives = controller().getAdjectives(str);
	for (std::vector<string>::const_iterator it = adjectives.begin(); it != adjectives.end(); ++it)
		fl_add_browser_line(adjective_->browser_adjective, it->c_str());

	std::vector<string> adverbs = controller().getAdverbs(str);
	for (std::vector<string>::const_iterator it = adverbs.begin(); it != adverbs.end(); ++it)
		fl_add_browser_line(adverb_->browser_adverb, it->c_str());

	std::vector<string> others = controller().getOthers(str);
	for (std::vector<string>::const_iterator it = others.begin(); it != others.end(); ++it)
		fl_add_browser_line(other_->browser_other, it->c_str());

	fl_unfreeze_form(noun_->form);
	fl_unfreeze_form(verb_->form);
	fl_unfreeze_form(adverb_->form);
	fl_unfreeze_form(adjective_->form);
	fl_unfreeze_form(other_->form);
	fl_redraw_form(noun_->form);
	fl_redraw_form(verb_->form);
	fl_redraw_form(adverb_->form);
	fl_redraw_form(adjective_->form);
	fl_redraw_form(other_->form);
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
	FL_OBJECT * browser = 0;

	if (obj == dialog_->input_entry) {
		updateEntries(fl_get_input(dialog_->input_entry));
		if (string(fl_get_input(dialog_->input_entry)).empty()) {
			fl_set_input(dialog_->input_replace, "");
			return ButtonPolicy::SMI_APPLY;
		}
	} else if (obj == dialog_->button_replace) {
		string rep(fl_get_input(dialog_->input_replace));
		if (!rep.empty())
			controller().replace(fl_get_input(dialog_->input_replace));
		clickline_ = -1;
		updateEntries(rep);
		return ButtonPolicy::SMI_APPLY;
	} else if (obj == noun_->browser_noun ||
		   obj == verb_->browser_verb ||
		   obj == adjective_->browser_adjective ||
		   obj == adverb_->browser_adverb ||
		   obj == other_->browser_other) {
		browser = obj;
	}

	if (browser) {
		setReplace(fl_get_input(dialog_->input_entry),
			fl_get_browser_line(browser, fl_get_browser(browser)));

		if (clickline_ == fl_get_browser(browser)) {
			updateEntries(fl_get_input(dialog_->input_replace));
			clickline_ = -1;
		} else {
			clickline_ = fl_get_browser(browser);
		}
	}

	return ButtonPolicy::SMI_VALID;
}
