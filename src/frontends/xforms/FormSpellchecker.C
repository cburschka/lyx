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
#include FORMS_H_LOCATION

typedef FormCB<ControlSpellchecker, FormDB<FD_spellchecker> > base_class;

FormSpellchecker::FormSpellchecker()
	: base_class(_("Spellchecker"))
{}


void FormSpellchecker::build()
{
	dialog_.reset(build_spellchecker(this));

	fl_set_browser_dblclick_callback(dialog_->browser,
					 C_FormBaseInputCB, 2);

	fl_set_input_return(dialog_->input, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input);

	// Manage the buttons
	bc().setCancel(dialog_->button_close);
	bc().addReadOnly(dialog_->button_replace);
	bc().addReadOnly(dialog_->button_accept);
	bc().addReadOnly(dialog_->button_add);
	bc().addReadOnly(dialog_->button_ignore);
	bc().addReadOnly(dialog_->button_start);
	bc().addReadOnly(dialog_->browser);

	// set up the tooltips
	string str = _("Type replacement for unknown word "
			" or select from suggestions.");
	tooltips().init(dialog_->input, str);
	str = _("List of replacement suggestions from dictionary.");
	tooltips().init(dialog_->browser, str);
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
}

void FormSpellchecker::update()
{
	fl_set_input(dialog_->input, "");
	fl_set_object_label(dialog_->text, "");
	fl_clear_browser(dialog_->browser);
	fl_set_slider_value(dialog_->slider, 0);
	start(true);
}

ButtonPolicy::SMInput FormSpellchecker::input(FL_OBJECT * obj, long val)
{
	if (obj == dialog_->button_replace) {
		string const tmp = getString(dialog_->input);
		controller().replace(tmp);

	} else if (obj == dialog_->button_start) {
		if (start())
			controller().check();
		else
			controller().stop();

	} else if (obj == dialog_->button_ignore) {
		controller().check();

	} else if (obj == dialog_->button_accept) {
		controller().ignoreAll();

	} else if (obj == dialog_->button_add) {
		controller().insert();

	} else if (obj == dialog_->browser) {
		int const line = fl_get_browser(dialog_->browser);
		string const tmp =
			getString(dialog_->browser, line);
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

bool FormSpellchecker::start(bool init)
{
	static bool running = false;

	if (init) {
		running = false;
	} else {
		running = !running;
	}

	fl_set_object_label(dialog_->button_start,
			(running ? _("Stop") : _("Start")));	
	fl_set_button_shortcut(dialog_->button_start, "#S", 1);
	fl_show_object(dialog_->button_start);

	string const str = (running ? _("Stop the spellingchecker.") :
					_("Start the spellingchecker."));
	tooltips().init(dialog_->button_start, str);

	setEnabled(dialog_->button_replace, running);
	setEnabled(dialog_->button_ignore, running);
	setEnabled(dialog_->button_accept, running);
	setEnabled(dialog_->button_add, running);
	setEnabled(dialog_->browser, running);
	setEnabled(dialog_->input, running);
	
	return running;
}
