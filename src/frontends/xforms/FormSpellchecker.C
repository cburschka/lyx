/**
 * \file FormSpellchecker.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormSpellchecker.h"
#include "ControlSpellchecker.h"
#include "forms/form_spellchecker.h"

#include "Tooltips.h"
#include "xforms_helpers.h"
#include "xformsBC.h"

#include "support/tostr.h"

#include "lyx_forms.h"

using std::endl;


typedef FormCB<ControlSpellchecker, FormDB<FD_spellchecker> > base_class;

FormSpellchecker::FormSpellchecker()
	: base_class(_("Spell-check Document"))
{}


void FormSpellchecker::build()
{
	dialog_.reset(build_spellchecker(this));

	// Manage the buttons
	bcview().setCancel(dialog_->button_close);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->button_replace);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_replacement);

	fl_set_input_return(dialog_->input_replacement, FL_RETURN_CHANGED);

	// callback for double click in browser
	fl_set_browser_dblclick_callback(dialog_->browser_suggestions,
					 C_FormBaseInputCB, 2);

	// do not allow setting of slider by the mouse
	fl_deactivate_object(dialog_->slider_progress);

	// set up the tooltips
	string str = _("Type replacement for unknown word "
			"or select from suggestions.");
	tooltips().init(dialog_->input_replacement, str);
	str = _("List of replacement suggestions from dictionary.");
	tooltips().init(dialog_->browser_suggestions, str);
	// Work-around xforms' bug; enable tooltips for browser widgets.
	setPrehandler(dialog_->browser_suggestions);
	str = _("Replace unknown word.");
	tooltips().init(dialog_->button_replace, str);
	str = _("Ignore unknown word.");
	tooltips().init(dialog_->button_ignore, str);
	str = _("Accept unknown word as known in this session.");
	tooltips().init(dialog_->button_accept, str);
	str = _("Add unknown word to personal dictionary.");
	tooltips().init(dialog_->button_add, str);
	str = _("Shows word count and progress on spell check.");
	tooltips().init(dialog_->slider_progress, str);
}


void FormSpellchecker::partialUpdate(int s)
{
	ControlSpellchecker::State const state =
		static_cast<ControlSpellchecker::State>(s);

	switch (state) {

	case ControlSpellchecker::SPELL_FOUND_WORD: {
		// Set suggestions.
		string w = controller().getWord();
		fl_set_input(dialog_->input_replacement, w.c_str());
		fl_set_object_label(dialog_->text_unknown, w.c_str());
		fl_clear_browser(dialog_->browser_suggestions);
		while (!(w = controller().getSuggestion()).empty()) {
			fl_add_browser_line(dialog_->browser_suggestions,
					    w.c_str());
		}
		// Fall through...
	}

	case ControlSpellchecker::SPELL_PROGRESSED: {
		int const progress = controller().getProgress();
		if (progress == 0)
			break;

		double const wordcount = controller().getCount();
		double const total = 100.0 * wordcount / progress;
		string const label = tostr(progress) + " %";

		fl_set_slider_bounds(dialog_->slider_progress, 0.0, total);
		fl_set_slider_value(dialog_->slider_progress, wordcount);
		fl_set_object_label(dialog_->slider_progress, label.c_str());
		fl_redraw_object(dialog_->slider_progress);
		break;
	}

	}
}


ButtonPolicy::SMInput FormSpellchecker::input(FL_OBJECT * ob, long ob_value)
{
	if (ob == dialog_->button_replace) {
		string const tmp = getString(dialog_->input_replacement);
		controller().replace(tmp);

	} else if (ob == dialog_->button_ignore) {
		controller().check();

	} else if (ob == dialog_->button_accept) {
		controller().ignoreAll();

	} else if (ob == dialog_->button_add) {
		controller().insert();

	} else if (ob == dialog_->browser_suggestions) {
		string const tmp = getString(dialog_->browser_suggestions);
		if (tmp.empty())
			return ButtonPolicy::SMI_NOOP;

		if (ob_value != 2) {
			// single-click
			// place the chosen string in the input as feedback
			fl_set_input(dialog_->input_replacement, tmp.c_str());

		} else {
			// double-click
			controller().replace(tmp);
			// reset the browser so that the following
			// single-click callback doesn't do anything
			fl_deselect_browser(dialog_->browser_suggestions);
		}
	}

	return ButtonPolicy::SMI_VALID;
}
