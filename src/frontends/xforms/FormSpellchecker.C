/**
 * \file FormSpellchecker.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "xforms_helpers.h"
#include "ControlSpellchecker.h"
#include "FormSpellchecker.h"
#include "form_spellchecker.h"

typedef FormCB<ControlSpellchecker, FormDB<FD_form_spellchecker> > base_class;

FormSpellchecker::FormSpellchecker(ControlSpellchecker & c)
	: base_class(c, _("LyX: Spellchecker"), false)
{}


void FormSpellchecker::build()
{
	dialog_.reset(build_spellchecker());
	
	fl_set_slider_bounds(dialog_->slider, 0.0, 100.0);
	fl_set_slider_step(dialog_->slider, 1.0);

	fl_set_browser_dblclick_callback(dialog_->browser,
					 C_FormBaseInputCB, 2);

	fl_set_input_return(dialog_->input, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input);

	// Manage the buttons
	bc().setCancel(dialog_->done);
	bc().addReadOnly(dialog_->replace);
	bc().addReadOnly(dialog_->accept);
	bc().addReadOnly(dialog_->insert);
	bc().addReadOnly(dialog_->ignore);
	bc().addReadOnly(dialog_->start);
	bc().addReadOnly(dialog_->stop);
	bc().addReadOnly(dialog_->browser);
}

void FormSpellchecker::update()
{
	string const w;
	fl_set_input(dialog_->input, w.c_str());
	fl_set_object_label(dialog_->text, w.c_str());
	fl_clear_browser(dialog_->browser);
	fl_set_slider_value(dialog_->slider, 0);
}

ButtonPolicy::SMInput FormSpellchecker::input(FL_OBJECT * obj, long val)
{
	if (obj == dialog_->replace) {
		string const tmp = getStringFromInput(dialog_->input);
		controller().replace(tmp);

	} else if (obj == dialog_->start) {
		controller().check();
		stop(false);

	} else if (obj == dialog_->stop) {
		controller().stop();
		stop(true);

	} else if (obj == dialog_->ignore) {
		controller().check();

	} else if (obj == dialog_->accept) {
		controller().ignoreAll();

	} else if (obj == dialog_->insert) {
		controller().insert();

	} else if (obj == dialog_->options) {
		controller().options();

	} else if (obj == dialog_->browser) {
		int const line = fl_get_browser(dialog_->browser);
		string const tmp =
			getStringFromBrowser(dialog_->browser, line);
		if (tmp.empty())
			return ButtonPolicy::SMI_NOOP;

		if (val != 2) {
			// single-click
			// place the chosen string in the input as feedback
			fl_set_input(dialog_->input, tmp.c_str());

		} else {
			// double-click
			controller().replace(tmp);
			// reset the browser so that the following
			// single-click callback doesn't do anything
			fl_deselect_browser_line(dialog_->browser, line);
		}
	}

	return ButtonPolicy::SMI_VALID;
}

void FormSpellchecker::partialUpdate(int id)
{
	switch (id) {
	case 0:
		// set progress bar
		fl_set_slider_value(dialog_->slider,
				    controller().getProgress());
		break;
	case 1:
	{
		// set suggestions
		string w = controller().getWord();
		fl_set_input(dialog_->input, w.c_str());
		fl_set_object_label(dialog_->text, w.c_str());
		fl_clear_browser(dialog_->browser);
		while (!(w = controller().getSuggestion()).empty()) {
			fl_add_browser_line(dialog_->browser, w.c_str());
		}
	}
		break;
	case 2:
		// show exit message
		fl_show_messages(controller().getMessage().c_str());
		hide();
	}
	
}

		
void FormSpellchecker::showMessage(const char * msg)
{
	fl_show_message(msg, "", "");
}

void FormSpellchecker::stop(bool stop)
{
	setEnabled(dialog_->start, stop);
	setEnabled(dialog_->replace, !stop);
	setEnabled(dialog_->ignore, !stop);
	setEnabled(dialog_->accept, !stop);
	setEnabled(dialog_->insert, !stop);
	setEnabled(dialog_->stop, !stop);
	setEnabled(dialog_->browser, !stop);
	setEnabled(dialog_->input, !stop);
}
