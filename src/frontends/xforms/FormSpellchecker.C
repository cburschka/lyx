/**
 * \file FormSpellchecker.C
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

#include "Tooltips.h"
#include "xformsBC.h"
#include "xforms_helpers.h"
#include "ControlSpellchecker.h"
#include "FormSpellchecker.h"
#include "forms/form_spellchecker.h"
#include "support/lstrings.h"

#include FORMS_H_LOCATION

typedef FormCB<ControlSpellchecker, FormDB<FD_spellchecker> > base_class;


FormSpellchecker::FormSpellchecker()
	: base_class(_("Spellchecker"))
{}


void FormSpellchecker::build()
{
	dialog_.reset(build_spellchecker(this));

	// Manage the buttons
	bc().setCancel(dialog_->button_close);

	// disable for read-only documents
	bc().addReadOnly(dialog_->button_replace);

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
			" or select from suggestions.");
	tooltips().init(dialog_->input_replacement, str);
	str = _("List of replacement suggestions from dictionary.");
	tooltips().init(dialog_->browser_suggestions, str);
	// Work-around xforms' bug; enable tooltips for browser widgets.
	setPrehandler(dialog_->browser_suggestions);
	str = _("Start the spellingchecker.");
	tooltips().init(dialog_->button_start, str);
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


void FormSpellchecker::update()
{
	// clear input fields
	fl_set_input(dialog_->input_replacement, "");
	fl_set_object_label(dialog_->text_unknown, "");
	fl_clear_browser(dialog_->browser_suggestions);

	// reset dialog and buttons into start condition
	input(0, 0);

	// reset slider to zero count
	fl_set_slider_value(dialog_->slider_progress, 0.0);
	fl_set_object_label(dialog_->slider_progress, "0 %");
}


ButtonPolicy::SMInput FormSpellchecker::input(FL_OBJECT * ob, long ob_value)
{
	if (!ob || ob == dialog_->button_start) {
		static bool running = false;

		// update running status of spellingchecker
		running = !running && ob == dialog_->button_start;

		// modify text of Start/Stop button according to running status
		string const labeltext = running ? _("Stop") : _("Start");
		fl_set_object_label(dialog_->button_start, labeltext.c_str());	
		fl_set_button_shortcut(dialog_->button_start, "#S", 1);
		fl_show_object(dialog_->button_start);

		// adjust tooltips to modified Start/Stop button
		string const str = (running ? _("Stop the spellingchecker.") :
					_("Start the spellingchecker."));
		tooltips().init(dialog_->button_start, str);

		// enable buttons according to running status
		setEnabled(dialog_->button_replace, running);
		setEnabled(dialog_->button_ignore, running);
		setEnabled(dialog_->button_accept, running);
		setEnabled(dialog_->button_add, running);
		setEnabled(dialog_->browser_suggestions, running);
		setEnabled(dialog_->input_replacement, running);

		// call controller if Start/Stop button is pressed
		if (ob) {
			if (running)
				controller().check();
			else
				controller().stop();
		}

	} else if (ob == dialog_->button_replace) {
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

	// update slider with word count and progress
	double const wordcount = controller().getCount();
	int const progress = controller().getProgress();
	if (wordcount > 0.0 && progress > 0) {
		double const total = 100.0 * wordcount / progress;

		fl_set_slider_bounds(dialog_->slider_progress, 0.0, total);
		fl_set_slider_value(dialog_->slider_progress, wordcount);
		fl_set_object_label(dialog_->slider_progress,
					(tostr(progress) + " %").c_str());
	}

	return ButtonPolicy::SMI_VALID;
}


void FormSpellchecker::partialUpdate(int id)
{
	switch (id) {
	case 1: // set suggestions
	{
		string w = controller().getWord();
		fl_set_input(dialog_->input_replacement, w.c_str());
		fl_set_object_label(dialog_->text_unknown, w.c_str());
		fl_clear_browser(dialog_->browser_suggestions);
		while (!(w = controller().getSuggestion()).empty()) {
			fl_add_browser_line(dialog_->browser_suggestions, w.c_str());
		}
	}
	break;
	case 2: // end of spell checking

		// reset dialog and buttons into start condition
		input(0, 0);

		// set slider 'finished' status
		fl_set_slider_bounds(dialog_->slider_progress, 0.0, controller().getCount());
		fl_set_slider_value(dialog_->slider_progress, controller().getCount());
		fl_set_object_label(dialog_->slider_progress, "100 %");

		break;
	}
}
