/**
 * \file xforms/FormCitation.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#include <config.h>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlCitation.h"
#include "FormCitation.h"
#include "form_citation.h"
#include "Tooltips.h"
#include "helper_funcs.h"
#include "xforms_helpers.h"
#include "gettext.h"
#include "support/LAssert.h"
#include "support/lstrings.h"

using std::find;
using std::max;
using std::min;
using std::pair;
using std::sort;
using std::vector;

namespace {

// shamelessly stolen from Menubar_pimpl.C
int string_width(string const & str) 
{
	return fl_get_string_widthTAB(FL_NORMAL_STYLE, FL_NORMAL_SIZE,
				      str.c_str(),
				      static_cast<int>(str.length()));
}


void fillChoice(FD_form_citation * dialog, vector<string> vec)
{
	// Check whether the current contents of the browser will be
	// changed by loading the contents of the vec...
	vector<string> const choice_style =
		getVectorFromChoice(dialog->choice_style);

	if (vec == choice_style)
		return;

	// They will be changed. Proceed
	string const str = " " + getStringFromVector(vec, " | ") + " ";

	fl_clear_choice(dialog->choice_style);
	fl_addto_choice(dialog->choice_style, str.c_str());

	// The width of the choice varies with the contents.
	// Ensure that it is centred in the frame.

	int width = 0;
	for (vector<string>::const_iterator it = vec.begin();
	     it != vec.end(); ++it) {
		width = max(width, string_width(*it));
	}

	int const dx =
		max(5, int(0.5 * (dialog->frame_style->w - width)));

	fl_set_object_geometry(dialog->choice_style,
			       dialog->frame_style->x + dx,
			       dialog->choice_style->y,
			       width,
			       dialog->choice_style->h);
}


void updateStyle(FD_form_citation * dialog, string command)
{
	// Find the style of the citekeys
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();
	biblio::CitationStyle cs = biblio::getCitationStyle(command);

	vector<biblio::CiteStyle>::const_iterator cit =
		find(styles.begin(), styles.end(), cs.style);

	// Use this to initialise the GUI
	if (cit == styles.end()) {
		fl_set_choice(dialog->choice_style, 1);
		fl_set_button(dialog->check_full_author_list, 0);
		fl_set_button(dialog->check_force_uppercase, 0);
	} else {
		int const i = int(cit - styles.begin());
		fl_set_choice(dialog->choice_style, i+1);
		fl_set_button(dialog->check_full_author_list,  cs.full);
		fl_set_button(dialog->check_force_uppercase, cs.forceUCase);
	}
}

} // namespace anon


typedef FormCB<ControlCitation, FormDB<FD_form_citation> > base_class;


FormCitation::FormCitation(ControlCitation & c)
	: base_class(c, _("Citation"), false)
{}


void FormCitation::apply()
{
	vector<biblio::CiteStyle> const & styles =
		ControlCitation::getCiteStyles();

	int const choice = fl_get_choice(dialog_->choice_style) - 1;
	bool const full  = fl_get_button(dialog_->check_full_author_list);
	bool const force = fl_get_button(dialog_->check_force_uppercase);

	string const command =
		biblio::getCiteCommand(styles[choice], full, force);

	controller().params().setCmdName(command);
	controller().params().setContents(getStringFromVector(citekeys));

	string const after  = fl_get_input(dialog_->input_after);
	controller().params().setOptions(after);
}


void FormCitation::hide()
{
	citekeys.clear();
	bibkeys.clear();

	FormBase::hide();
}


void FormCitation::build()
{
	dialog_.reset(build_citation());

	fl_set_input_return(dialog_->input_after,  FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_before, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_search, FL_RETURN_END);

	fl_set_button(dialog_->check_search_case, 0);
	fl_set_button(dialog_->check_search_type, 0);

	setPrehandler(dialog_->input_search);
	setPrehandler(dialog_->input_before);
	setPrehandler(dialog_->input_after);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->button_add);
	bc().addReadOnly(dialog_->button_del);
	bc().addReadOnly(dialog_->button_up);
	bc().addReadOnly(dialog_->button_down);
	bc().addReadOnly(dialog_->choice_style);
	bc().addReadOnly(dialog_->input_before);
	bc().addReadOnly(dialog_->input_after);
	bc().addReadOnly(dialog_->check_full_author_list);
	bc().addReadOnly(dialog_->check_force_uppercase);

	//set up the tooltip mechanism
	string str = N_("Add the selected entry to the current citation reference.");
	tooltips().init(dialog_->button_add, str);

	str = N_("Delete the selected entry from the current citation reference.");
	tooltips().init(dialog_->button_del, str);

	str = N_("Move the selected entry upwards (in the current list).");
	tooltips().init(dialog_->button_up, str);

	str = N_("Move the selected entry downwards (in the current list).");
	tooltips().init(dialog_->button_down, str);

	str = N_("The entries which will be cited. Select them with the arrow buttons from the right browser window.");
	tooltips().init(dialog_->browser_cite, str);

	str = N_("All entries in the database you have loaded (via \"Insert->Lists&TOC->BibTex Reference\"). Move the ones you want to cite with the arrow buttons into the left browser window.");
	tooltips().init(dialog_->browser_bib, str);

	str = N_("Information about the selected entry");
	tooltips().init(dialog_->browser_info, str);

	str = N_("Here you may select how the citation label should look inside the text (Natbib).");
	tooltips().init(dialog_->choice_style, str);

	str = N_("Activate if you want to print all authors in a reference with more than three authors, and not \"<First Author> et.al.\" (Natbib).");
	tooltips().init(dialog_->check_full_author_list, str);

	str = N_("Activate if you want to print the first character of the author name as uppercase (\"Van Gogh\", not \"van Gogh\"). Useful at the beginning of sentences (Natbib).");
	tooltips().init(dialog_->check_force_uppercase, str);

	str = N_("Optional text which appears before the citation reference, e.g. \"see <Ref>\"");
	tooltips().init(dialog_->input_before, str);

	str = N_("Optional text which appears after the citation reference, e.g. \"pp. 12\"");
	tooltips().init(dialog_->input_after, str);

	str = N_("Search your database (all fields will be searched).");
	tooltips().init(dialog_->input_search, str);

	str = N_("Activate if you want to have case sensitive search: \"bibtex\" finds \"bibtex\", but not \"BibTeX\".");
	tooltips().init(dialog_->check_search_case, str);

	str = N_("Activate if you want to enter Regular Expressions.");
	tooltips().init(dialog_->check_search_type, str);
}


void FormCitation::findBiblio(biblio::Direction const dir)
{
	string const str = fl_get_input(dialog_->input_search);
	biblio::InfoMap const & theMap = controller().bibkeysInfo();
	bool const caseSensitive =
		fl_get_button(dialog_->check_search_case);
	biblio::Search const type =
		fl_get_button(dialog_->check_search_type) ?
		biblio::REGEX : biblio::SIMPLE;

	vector<string>::const_iterator start = bibkeys.begin();
	int const sel = fl_get_browser(dialog_->browser_bib);
	if (sel >= 1 && sel <= int(bibkeys.size()))
		start += sel - 1;

	// Find the NEXT instance...
	(dir == biblio::FORWARD) ? ++start : --start;


	vector<string>::const_iterator const cit =
		biblio::searchKeys(theMap, bibkeys, str,
				   start, type, dir, caseSensitive);

	if (cit == bibkeys.end())
		return;

	int const found = int(cit - bibkeys.begin()) + 1;
	if (found == sel)
		return;

	// Update the display
	int const top = max(found - 5, 1);
	fl_set_browser_topline(dialog_->browser_bib, top);
	fl_select_browser_line(dialog_->browser_bib, found);
	input(dialog_->browser_bib, 0);
}
 

ButtonPolicy::SMInput FormCitation::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	biblio::InfoMap const & theMap = controller().bibkeysInfo();

	string topCitekey;
	if (!citekeys.empty()) topCitekey = citekeys[0];

	if (ob == dialog_->browser_bib) {
		fl_deselect_browser(dialog_->browser_cite);

		unsigned int const sel = fl_get_browser(dialog_->browser_bib);
		if (sel < 1 || sel > bibkeys.size())
			return ButtonPolicy::SMI_NOOP;

		// Put into browser_info the additional info associated with
		// the selected browser_bib key
		fl_clear_browser(dialog_->browser_info);

		string const tmp = formatted(biblio::getInfo(theMap,
							     bibkeys[sel-1]),
					     dialog_->browser_info->w-10);
		fl_add_browser_line(dialog_->browser_info, tmp.c_str());

		// Highlight the selected browser_bib key in browser_cite if
		// present
		vector<string>::const_iterator cit =
			find(citekeys.begin(), citekeys.end(), bibkeys[sel-1]);

		if (cit != citekeys.end()) {
			int const n = int(cit - citekeys.begin());
			fl_select_browser_line(dialog_->browser_cite, n+1);
			fl_set_browser_topline(dialog_->browser_cite, n+1);
		}

		if (!controller().isReadonly()) {
			if (cit != citekeys.end()) {
				setBibButtons(OFF);
				setCiteButtons(ON);
			} else {
				setBibButtons(ON);
				setCiteButtons(OFF);
			}
		}

	} else if (ob == dialog_->browser_cite) {
		unsigned int const sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 1 || sel > citekeys.size())
			return ButtonPolicy::SMI_NOOP;

		if (!controller().isReadonly()) {
			setBibButtons(OFF);
			setCiteButtons(ON);
		}

		// Highlight the selected browser_cite key in browser_bib
		vector<string>::const_iterator cit =
			find(bibkeys.begin(), bibkeys.end(), citekeys[sel-1]);

		if (cit != bibkeys.end()) {
			int const n = int(cit - bibkeys.begin());
			fl_select_browser_line(dialog_->browser_bib, n+1);
			fl_set_browser_topline(dialog_->browser_bib, n+1);

			// Put into browser_info the additional info associated
			// with the selected browser_cite key
			fl_clear_browser(dialog_->browser_info);
			string const tmp =
				formatted(biblio::getInfo(theMap,
							  citekeys[sel-1]),
					  dialog_->browser_info->w-10);
			fl_add_browser_line(dialog_->browser_info, tmp.c_str());
		}

	} else if (ob == dialog_->button_add) {
		unsigned int const sel = fl_get_browser(dialog_->browser_bib);
		if (sel < 1 || sel > bibkeys.size())
			return ButtonPolicy::SMI_NOOP;

		// Add the selected browser_bib key to browser_cite
		fl_addto_browser(dialog_->browser_cite,
				 bibkeys[sel-1].c_str());
		citekeys.push_back(bibkeys[sel-1]);

		int const n = int(citekeys.size());
		fl_select_browser_line(dialog_->browser_cite, n);

		setBibButtons(OFF);
		setCiteButtons(ON);
		activate = ButtonPolicy::SMI_VALID;

	} else if (ob == dialog_->button_del) {
		unsigned int const sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 1 || sel > citekeys.size())
			return ButtonPolicy::SMI_NOOP;

		// Remove the selected key from browser_cite
		fl_delete_browser_line(dialog_->browser_cite, sel) ;
		citekeys.erase(citekeys.begin() + sel-1);

		setBibButtons(ON);
		setCiteButtons(OFF);
		activate = ButtonPolicy::SMI_VALID;

	} else if (ob == dialog_->button_up) {
		unsigned int const sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 2 || sel > citekeys.size())
			return ButtonPolicy::SMI_NOOP;

		// Move the selected key up one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string const tmp = *it;

		fl_delete_browser_line(dialog_->browser_cite, sel);
		citekeys.erase(it);

		fl_insert_browser_line(dialog_->browser_cite, sel-1, tmp.c_str());
		fl_select_browser_line(dialog_->browser_cite, sel-1);
		citekeys.insert(it-1, tmp);
		setCiteButtons(ON);
		activate = ButtonPolicy::SMI_VALID;

	} else if (ob == dialog_->button_down) {
		unsigned int const sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 1 || sel > citekeys.size()-1)
			return ButtonPolicy::SMI_NOOP;

		// Move the selected key down one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string const tmp = *it;

		fl_delete_browser_line(dialog_->browser_cite, sel);
		citekeys.erase(it);

		fl_insert_browser_line(dialog_->browser_cite, sel+1, tmp.c_str());
		fl_select_browser_line(dialog_->browser_cite, sel+1);
		citekeys.insert(it+1, tmp);
		setCiteButtons(ON);
		activate = ButtonPolicy::SMI_VALID;

	} else if (ob == dialog_->button_previous) {
		findBiblio(biblio::BACKWARD);
	} else if (ob == dialog_->button_next) {
		findBiblio(biblio::FORWARD);
	} else if (ob == dialog_->input_search) {
		findBiblio(biblio::FORWARD);
	} else if (ob == dialog_->choice_style ||
		   ob == dialog_->check_full_author_list ||
		   ob == dialog_->check_force_uppercase ||
		   ob == dialog_->input_before ||
		   ob == dialog_->input_after) {
		activate = ButtonPolicy::SMI_VALID;
	}

	string currentCitekey;
	if (!citekeys.empty())
		currentCitekey = citekeys[0];

	if (topCitekey != currentCitekey) {
		int choice = fl_get_choice(dialog_->choice_style);
		fillChoice(dialog_.get(),
			   controller().getCiteStrings(currentCitekey));
		fl_set_choice(dialog_->choice_style, choice);
	}

	return activate;
}


void FormCitation::update()
{
	// Make the list of all available bibliography keys
	bibkeys = biblio::getKeys(controller().bibkeysInfo());
	updateBrowser(dialog_->browser_bib, bibkeys);

	// Ditto for the keys cited in this inset
	citekeys = getVectorFromString(controller().params().getContents());
	updateBrowser(dialog_->browser_cite, citekeys);

	// Use the first citekey to fill choice_style
	string key;
	if (!citekeys.empty()) key = citekeys[0];

	fillChoice(dialog_.get(), controller().getCiteStrings(key));

	// Use the citation command to update the GUI
	updateStyle(dialog_.get(), controller().params().getCmdName());

	bool const natbib = controller().usingNatbib();
	setEnabled(dialog_->check_full_author_list, natbib);
	setEnabled(dialog_->check_force_uppercase, natbib);
	setEnabled(dialog_->choice_style, natbib);
	
	// No keys have been selected yet, so...
	fl_clear_browser(dialog_->browser_info);
	setBibButtons(OFF);
	setCiteButtons(OFF);

	// Natbib can have comments before and after the citation.
	// This is not yet supported. After only.
	fl_set_input(dialog_->input_after,
		     controller().params().getOptions().c_str());

	fl_set_input(dialog_->input_before, _("Not yet supported"));
	setEnabled(dialog_->input_before, false);
}


void FormCitation::updateBrowser(FL_OBJECT * browser,
				 vector<string> const & keys) const
{
	// Check whether the current contents of the browser will be
	// changed by loading the contents of the vec...
	vector<string> browser_keys = getVectorFromBrowser(browser);

	if (browser_keys == keys)
		return;

	// They will be changed. Proceed.
	fl_clear_browser(browser);

	for (vector<string>::const_iterator it = keys.begin();
	     it < keys.end(); ++it) {
		string key = frontStrip(strip(*it));
		if (!key.empty())
			fl_add_browser_line(browser, key.c_str());
	}
}


void FormCitation::setBibButtons(State status) const
{
	setEnabled(dialog_->button_add, (status == ON));
}


void FormCitation::setCiteButtons(State status) const
{
	int const sel     = fl_get_browser(dialog_->browser_cite);
	int const maxline = fl_get_browser_maxline(dialog_->browser_cite);
	bool const activate      = (status == ON);
	bool const activate_up   = (activate && sel != 1);
	bool const activate_down = (activate && sel != maxline);

	setEnabled(dialog_->button_del,  activate);
	setEnabled(dialog_->button_up,   activate_up);
	setEnabled(dialog_->button_down, activate_down);
}
