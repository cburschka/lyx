// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"
#include "FormCitation.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_citation.h"
#include "lyxfunc.h"
#include "support/filetools.h"

using std::find;
using std::max;
using std::min;
using std::pair;
using std::sort;
using std::vector;


FormCitation::FormCitation(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Citation"), new NoRepeatedApplyReadOnlyPolicy),
	  dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCitation.connect(slot(this, &FormCitation::showInset));
	d->createCitation.connect(slot(this, &FormCitation::createInset));
}


FormCitation::~FormCitation()
{
	delete dialog_;
}


FL_FORM * FormCitation::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
}


void FormCitation::connect()
{
	//fl_set_form_maxsize(dialog_->form, 3*minw_, minh_);
	FormCommand::connect();
}


void FormCitation::disconnect()
{
	citekeys.clear();
	bibkeys.clear();
	bibkeysInfo.clear();

	FormCommand::disconnect();
}


void FormCitation::build()
{
	dialog_ = build_citation();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_input_return(dialog_->input_after,  FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_before, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.setUndoAll(dialog_->button_restore);
	bc_.refresh();

	bc_.addReadOnly(dialog_->button_add);
	bc_.addReadOnly(dialog_->button_del);
	bc_.addReadOnly(dialog_->button_up);
	bc_.addReadOnly(dialog_->button_down);
	bc_.addReadOnly(dialog_->input_before);
	bc_.addReadOnly(dialog_->input_after);

	bc_.addDontTriggerChange(dialog_->browser_cite);
	bc_.addDontTriggerChange(dialog_->browser_bib);
}


void FormCitation::update()
{
	bc_.readOnly(lv_->buffer()->isReadonly());

	bibkeys.clear();
	bibkeysInfo.clear();

	vector<pair<string,string> > blist =
		lv_->buffer()->getBibkeyList();
	sort(blist.begin(), blist.end());

	for (unsigned int i = 0; i < blist.size(); ++i) {
		bibkeys.push_back(blist[i].first);
		bibkeysInfo.push_back(blist[i].second);
	}
	blist.clear();
	updateBrowser(dialog_->browser_bib, bibkeys);

	// Ditto for the keys cited in this inset
	citekeys.clear();
	string tmp, keys(params.getContents());
	keys = frontStrip(split(keys, tmp, ','));
	while (!tmp.empty()) {
		citekeys.push_back(tmp);
		keys = frontStrip(split(keys, tmp, ','));
	}
	updateBrowser(dialog_->browser_cite, citekeys);

	// No keys have been selected yet, so...
	fl_clear_browser(dialog_->browser_info);
	setBibButtons(OFF);
	setCiteButtons(OFF);

	int noKeys = static_cast<int>(max(bibkeys.size(), citekeys.size()));

	// Place bounds, so that 4 <= noKeys <= 10
	noKeys = max(4, min(10, noKeys));

	// Re-size the form to accommodate the new browser size
	int size = 20 * noKeys;
	bool bibPresent = (bibkeys.size() > 0);
	setSize(size, bibPresent);

	fl_set_input(dialog_->input_after, params.getOptions().c_str());
}


void FormCitation::updateBrowser(FL_OBJECT * browser,
				  vector<string> const & keys) const
{
	fl_clear_browser(browser);

	for (unsigned int i = 0; i < keys.size(); ++i)
		fl_add_browser_line(browser, keys[i].c_str());
}


void FormCitation::setBibButtons(State status) const
{
	switch (status) {
	case ON:
		fl_activate_object(dialog_->button_add);
		fl_set_object_lcol(dialog_->button_add, FL_BLACK);
		break;

	case OFF:
		fl_deactivate_object(dialog_->button_add);
		fl_set_object_lcol(dialog_->button_add, FL_INACTIVE);
		break;

	default:
		break;
	}
}


void FormCitation::setCiteButtons(State status) const
{
	switch (status) {
	case ON:
        {
		fl_activate_object(dialog_->button_del);
		fl_set_object_lcol(dialog_->button_del, FL_BLACK);

		int sel = fl_get_browser(dialog_->browser_cite);

		if (sel != 1) {
			fl_activate_object(dialog_->button_up);
			fl_set_object_lcol(dialog_->button_up, FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->button_up);
			fl_set_object_lcol(dialog_->button_up, FL_INACTIVE);
		}

		if (sel != fl_get_browser_maxline(dialog_->browser_cite)) {
			fl_activate_object(dialog_->button_down);
			fl_set_object_lcol(dialog_->button_down, FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->button_down);
			fl_set_object_lcol(dialog_->button_down, FL_INACTIVE);
		}

		break;
	}
	case OFF:
	{
		fl_deactivate_object(dialog_->button_del);
		fl_set_object_lcol(dialog_->button_del, FL_INACTIVE);

		fl_deactivate_object(dialog_->button_up);
		fl_set_object_lcol(dialog_->button_up, FL_INACTIVE);

		fl_deactivate_object(dialog_->button_down);
		fl_set_object_lcol(dialog_->button_down, FL_INACTIVE);
	}
	default:
		break;
	}
}


void FormCitation::setSize(int hbrsr, bool bibPresent) const
{
	bool const natbib = false; // will eventually be input
	hbrsr = max(hbrsr, 175); // limit max size of cite/bib brsrs

	// dh1, dh2, dh3 are the vertical separation between elements.
	// These can be specified because the browser height is fixed
	// so they are not changed by dynamic resizing
	static int const dh1 = 30; // top of form to top of cite/bib brsrs;
	                           // bottom of cite/bib brsrs to top of info;
	                           // bottom of info to top next element;
	                           // bottom of style to top input_before;
	                           // bottom of text to top ok/cancel buttons.
	static int const dh2 = 10; // bottom of input_before to top input_after;
	                           // bottom of ok/cancel buttons to bottom form
	static int const dh3 = 5;  // spacing between add/delete/... buttons.

	int const wbrsr  = dialog_->browser_cite->w;
	static int const hinfo  = dialog_->browser_info->h;
	static int const hstyle = dialog_->choice_style->h;
	static int const htext  = dialog_->input_after->h;
	static int const hok    = dialog_->button_ok->h;

	int hform = dh1 + hbrsr + dh1;
	if (bibPresent) hform += hinfo + dh1;
	if (natbib) hform += hstyle + dh1 + htext + dh2;
	hform += htext + dh1 + hok + dh2;

	if (hform != minh_) {
		minh_ = hform;
		fl_set_form_size(dialog_->form, minw_, minh_);
	} else
		return;

	int x = 0;
	int y = 0;
	fl_set_object_geometry(dialog_->box, x, y, minw_, minh_);

	x = dialog_->browser_cite->x;
	y += dh1; 
	fl_set_object_geometry(dialog_->browser_cite, x, y, wbrsr, hbrsr);
	x = dialog_->browser_bib->x;
	fl_set_object_geometry(dialog_->browser_bib,  x, y, wbrsr, hbrsr);

	x = dialog_->button_add->x;
	fl_set_object_position(dialog_->button_add,  x, y);
	y += dh3 + dialog_->button_add->h;
	fl_set_object_position(dialog_->button_del,  x, y);
	y += dh3 + dialog_->button_del->h;
	fl_set_object_position(dialog_->button_up,   x, y);
	y += dh3 + dialog_->button_up->h;
	fl_set_object_position(dialog_->button_down, x, y);

	y = dh1 + hbrsr + dh1; // in position for next element

	if (bibPresent) {
		x = dialog_->browser_info->x;
		fl_set_object_position(dialog_->browser_info, x, y);
		fl_show_object(dialog_->browser_info);
		y += hinfo + dh1;
	} else
		fl_hide_object(dialog_->browser_info);

	if (natbib) {
		x = dialog_->choice_style->x;
		fl_set_object_position(dialog_->choice_style, x, y);
		fl_show_object(dialog_->choice_style);
		x = dialog_->input_before->x;
		y += hstyle + dh1;
		fl_set_object_position(dialog_->input_before, x, y);
		fl_show_object(dialog_->input_before);
		y += htext + dh2;
	} else {
		fl_hide_object(dialog_->choice_style);
		fl_hide_object(dialog_->input_before);
	}

	x = dialog_->input_after->x;
	fl_set_object_position(dialog_->input_after, x, y);

	y += htext + dh1;
	x = dialog_->button_restore->x;
	fl_set_object_position(dialog_->button_restore,     x, y);
	x = dialog_->button_ok->x;
	fl_set_object_position(dialog_->button_ok,     x, y);
	x = dialog_->button_apply->x;
	fl_set_object_position(dialog_->button_apply,  x, y);
	x = dialog_->button_cancel->x;
	fl_set_object_position(dialog_->button_cancel, x, y);
}


bool FormCitation::input(FL_OBJECT * ob, long)
{
	bool activate = false;

	if (ob == dialog_->browser_bib) {
		fl_deselect_browser(dialog_->browser_cite);
		
		unsigned int sel = fl_get_browser(dialog_->browser_bib);
		if (sel < 1 || sel > bibkeys.size()) return false;

		// Put into browser_info the additional info associated with
		// the selected browser_bib key
		fl_clear_browser(dialog_->browser_info);
		fl_add_browser_line(dialog_->browser_info,
				     bibkeysInfo[sel - 1].c_str());

		// Highlight the selected browser_bib key in browser_cite if present
		vector<string>::iterator it =
			find(citekeys.begin(), citekeys.end(), bibkeys[sel-1]);

		if (it != citekeys.end()) {
			int n = static_cast<int>(it - citekeys.begin());
			fl_select_browser_line(dialog_->browser_cite, n+1);
			fl_set_browser_topline(dialog_->browser_cite, n+1);
		}

		if (!lv_->buffer()->isReadonly()) {
			if (it != citekeys.end()) {
				setBibButtons(OFF);
				setCiteButtons(ON);
			} else {
				setBibButtons(ON);
				setCiteButtons(OFF);
			}
		}

	} else if (ob == dialog_->browser_cite) {
		unsigned int sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 1 || sel > citekeys.size()) return false;

		if (!lv_->buffer()->isReadonly()) {
			setBibButtons(OFF);
			setCiteButtons(ON);
		}

		// Highlight the selected browser_cite key in browser_bib
		vector<string>::iterator it =
			find(bibkeys.begin(), bibkeys.end(), citekeys[sel-1]);

		if (it != bibkeys.end()) {
			int n = static_cast<int>(it - bibkeys.begin());
			fl_select_browser_line(dialog_->browser_bib, n+1);
			fl_set_browser_topline(dialog_->browser_bib, n+1);

			// Put into browser_info the additional info associated with
			// the selected browser_cite key
			fl_clear_browser(dialog_->browser_info);
			fl_add_browser_line(dialog_->browser_info,
					     bibkeysInfo[n].c_str());
		}

	} else if (ob == dialog_->button_add) {
		unsigned int sel = fl_get_browser(dialog_->browser_bib);
		if (sel < 1 || sel > bibkeys.size()) return false;

		// Add the selected browser_bib key to browser_cite
		fl_addto_browser(dialog_->browser_cite,
				  bibkeys[sel-1].c_str());
		citekeys.push_back(bibkeys[sel-1]);

		int n = static_cast<int>(citekeys.size());
		fl_select_browser_line(dialog_->browser_cite, n);

		setBibButtons(OFF);
		setCiteButtons(ON);
		activate = true;

	} else if (ob == dialog_->button_del) {
		unsigned int sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 1 || sel > citekeys.size()) return false;

		// Remove the selected key from browser_cite
		fl_delete_browser_line(dialog_->browser_cite, sel) ;
		citekeys.erase(citekeys.begin() + sel-1);

		setBibButtons(ON);
		setCiteButtons(OFF);
		activate = true;

	} else if (ob == dialog_->button_up) {
		unsigned int sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 2 || sel > citekeys.size()) return false;

		// Move the selected key up one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line(dialog_->browser_cite, sel);
		citekeys.erase(it);

		fl_insert_browser_line(dialog_->browser_cite, sel-1, tmp.c_str());
		fl_select_browser_line(dialog_->browser_cite, sel-1);
		citekeys.insert(it-1, tmp);
		setCiteButtons(ON);
		activate = true;

	} else if (ob == dialog_->button_down) {
		unsigned int sel = fl_get_browser(dialog_->browser_cite);
		if (sel < 1 || sel > citekeys.size()-1) return false;

		// Move the selected key down one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line(dialog_->browser_cite, sel);
		citekeys.erase(it);

		fl_insert_browser_line(dialog_->browser_cite, sel+1, tmp.c_str());
		fl_select_browser_line(dialog_->browser_cite, sel+1);
		citekeys.insert(it+1, tmp);
		setCiteButtons(ON);
		activate = true;
	} else if (ob == dialog_->choice_style ||
		   ob == dialog_->input_before ||
		   ob == dialog_->input_after) {
		activate = true;
	}
	return activate;
}


void FormCitation::apply()
{
	if (lv_->buffer()->isReadonly()) return;

	string contents;
	for (unsigned int i = 0; i < citekeys.size(); ++i) {
		if (i > 0) contents += ", ";
		contents += citekeys[i];
	}

	params.setContents(contents);
	params.setOptions(fl_get_input(dialog_->input_after));

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else {
		lv_->getLyXFunc()->Dispatch(LFUN_CITATION_INSERT,
					    params.getAsString());
	}
}
