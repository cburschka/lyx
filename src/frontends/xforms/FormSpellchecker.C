/**
 * \file FormSpellchecker.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
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
	: base_class(c, _("LyX: Spellchecker")), clickline_(-1)
{}


void FormSpellchecker::build()
{
	dialog_.reset(build_spellchecker());
	
	fl_set_slider_bounds(dialog_->slider, 0.0, 100.0);
	fl_set_slider_step(dialog_->slider, 1.0);

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
	string w = "";
	fl_set_input(dialog_->input, w.c_str());
	fl_set_object_label(dialog_->text, w.c_str());
	fl_clear_browser(dialog_->browser);
	fl_set_slider_value(dialog_->slider, 0);
}

void FormSpellchecker::hide()
{
	clickline_ = -1;
	
	if (form() && form()->visible)
		fl_hide_form(form());
}

ButtonPolicy::SMInput FormSpellchecker::input(FL_OBJECT * obj, long)
{
	if (obj == dialog_->replace) {
		string const tmp = fl_get_input(dialog_->input);
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
		int const sel = fl_get_browser(dialog_->browser);
		if (sel < 1)
			return ButtonPolicy::SMI_NOOP;

		if (clickline_ == sel) {
			string const tmp = fl_get_input(dialog_->input);
			controller().replace(tmp);
		}

		clickline_ = sel;
		char const * cptmp = fl_get_browser_line(dialog_->browser,
							 clickline_);
		string const tmp = (cptmp) ? cptmp : "";
		fl_set_input(dialog_->input, tmp.c_str());
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
		while ( !(w = controller().getSuggestion()).empty() ) {
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
