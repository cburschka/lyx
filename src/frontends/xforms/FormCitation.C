/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormCitation.C
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
#include "gettext.h"
#include "support/lstrings.h"
#include "biblio.h"
#include "helper_funcs.h"
#include "xforms_helpers.h"

using std::find;
using std::max;
using std::min;
using std::pair;
using std::sort;
using std::vector;

typedef FormCB<ControlCitation, FormDB<FD_form_citation> > base_class;

FormCitation::FormCitation(ControlCitation & c)
	: base_class(c, _("Citation"))
{}


void FormCitation::apply()
{
	controller().params().setCmdName("cite");
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

	fl_set_button(dialog_->button_search_case, 0);
	fl_set_button(dialog_->button_search_type, 0);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setUndoAll(dialog_->button_restore);

	bc().addReadOnly(dialog_->button_add);
	bc().addReadOnly(dialog_->button_del);
	bc().addReadOnly(dialog_->button_up);
	bc().addReadOnly(dialog_->button_down);
	bc().addReadOnly(dialog_->choice_style);
	bc().addReadOnly(dialog_->input_before);
	bc().addReadOnly(dialog_->input_after);

	bc().refresh();
}


ButtonPolicy::SMInput FormCitation::input(FL_OBJECT * ob, long)
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	biblio::InfoMap const & theMap = controller().bibkeysInfo();

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
					      dialog_->browser_info->w-10 );
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
							  bibkeys[sel-1]),
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

	} else if (ob == dialog_->button_previous ||
		   ob == dialog_->button_next) {

		string const str = fl_get_input(dialog_->input_search);

		biblio::Direction const dir =
			(ob == dialog_->button_previous) ?
			biblio::BACKWARD : biblio::FORWARD;

		biblio::Search const type =
			fl_get_button(dialog_->button_search_type) ?
			biblio::REGEX : biblio::SIMPLE;

		vector<string>::const_iterator start = bibkeys.begin();
		int const sel = fl_get_browser(dialog_->browser_bib);
		if (sel >= 1 && sel <= int(bibkeys.size()))
			start += sel-1;

		// Find the NEXT instance...
		if (dir == biblio::FORWARD)
			start += 1;
		else
			start -= 1;

		bool const caseSensitive =
			fl_get_button(dialog_->button_search_case);

		vector<string>::const_iterator const cit =
			biblio::searchKeys(theMap, bibkeys, str,
					   start, type, dir, caseSensitive);

		if (cit == bibkeys.end())
			return ButtonPolicy::SMI_NOOP;

		int const found = int(cit - bibkeys.begin()) + 1;
		if (found == sel)
			return ButtonPolicy::SMI_NOOP;

		// Update the display
		int const top = max(found-5, 1);
		fl_set_browser_topline(dialog_->browser_bib, top);
		fl_select_browser_line(dialog_->browser_bib, found);
		input(dialog_->browser_bib, 0);

	} else if (ob == dialog_->choice_style ||
		   ob == dialog_->input_before ||
		   ob == dialog_->input_after) {
		activate = ButtonPolicy::SMI_VALID;
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

	// No keys have been selected yet, so...
	fl_clear_browser(dialog_->browser_info);
	setBibButtons(OFF);
	setCiteButtons(OFF);

	int noKeys = int(max(bibkeys.size(), citekeys.size()));

	// Place bounds, so that 4 <= noKeys <= 10
	noKeys = max(4, min(10, noKeys));

	// Re-size the form to accommodate the new browser size
	int const size = 20 * noKeys;
	bool const bibPresent = (bibkeys.size() > 0);
	setSize(size, bibPresent);

	fl_set_input(dialog_->input_after,
		     controller().params().getOptions().c_str());
}


void FormCitation::updateBrowser(FL_OBJECT * browser,
				 vector<string> const & keys) const
{
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


void FormCitation::setSize(int hbrsr, bool bibPresent) const
{
	bool const natbib = false; // will eventually be input
	hbrsr = max(hbrsr, 175); // limit max size of cite/bib brsrs

	// dh1, dh2, dh3 are the vertical separation between elements.
	// These can be specified because the browser height is fixed
	// so they are not changed by dynamic resizing
	static int const dh1 = 30; // top of form to top of cite/bib brsrs;
	                           // bottom of cite/bib brsrs to top of info;
	                           // bottom of info to top search frame;
	                           // bottom of search frame to top next elemnt;
	                           // bottom of style to top input_before;
	                           // bottom of text to top ok/cancel buttons.
	static int const dh2 = 10; // bottom of input_before to top input_after;
	                           // bottom of ok/cancel buttons to bottom form
	static int const dh3 = 5;  // spacing between add/delete/... buttons.

	int const wbrsr  = dialog_->browser_cite->w;
	static int const hinfo  = dialog_->browser_info->h;
	static int const hframe = dialog_->frame_search->h;
	static int const hstyle = dialog_->choice_style->h;
	static int const htext  = dialog_->input_after->h;
	static int const hok    = dialog_->button_ok->h;

	int hform = dh1 + hbrsr + dh1 + hframe + dh1;
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

	x = dialog_->frame_search->x;
	// ??? The frame height seems to be reduced. Use geometry to enforce it.
	fl_set_object_geometry(dialog_->frame_search, x, y,
			       dialog_->frame_search->w, hframe);
	//fl_set_object_position(dialog_->frame_search, x, y);

	x = dialog_->input_search->x;
	y += 15;
	fl_set_object_position(dialog_->input_search, x, y);

	x = dialog_->button_previous->x;
	y += dialog_->input_search->h + 5;
	fl_set_object_position(dialog_->button_previous, x, y);

	x = dialog_->button_next->x;
	y += dialog_->button_previous->h + 5;
	fl_set_object_position(dialog_->button_next, x, y);

	x = dialog_->button_search_type->x;
	y = dialog_->button_previous->y;
	fl_set_object_position(dialog_->button_search_type, x, y);

	x = dialog_->button_search_case->x;
	y = dialog_->button_next->y;
	fl_set_object_position(dialog_->button_search_case, x, y);

	y = dialog_->frame_search->y + hframe + dh1;
	
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
	fl_set_object_position(dialog_->button_restore, x, y);
	x = dialog_->button_ok->x;
	fl_set_object_position(dialog_->button_ok, x, y);
	x = dialog_->button_apply->x;
	fl_set_object_position(dialog_->button_apply, x, y);
	x = dialog_->button_cancel->x;
	fl_set_object_position(dialog_->button_cancel, x, y);
}
