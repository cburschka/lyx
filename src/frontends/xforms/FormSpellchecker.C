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
	
	// Manage the buttons
	bc().setCancel(dialog_->done);
	bc().addReadOnly(dialog_->replace);
	bc().addReadOnly(dialog_->accept);
	bc().addReadOnly(dialog_->insert);
	bc().addReadOnly(dialog_->ignore);
	bc().addReadOnly(dialog_->start);
	bc().addReadOnly(dialog_->stop);
}

void FormSpellchecker::update()
{
	string w = "";
	fl_set_input(dialog_->input, w.c_str());
	fl_set_object_label(dialog_->text, w.c_str());
	fl_clear_browser(dialog_->browser);
	fl_set_slider_value(dialog_->slider, 0);
	clickline_ = -1 ;
}


ButtonPolicy::SMInput FormSpellchecker::input(FL_OBJECT * obj, long)
{
	if (obj == dialog_->replace) {
		string tmp = fl_get_input(dialog_->input);
		controller().replace(tmp);
	} else if (obj == dialog_->start) {
		controller().check();
		fl_deactivate_object(dialog_->start);
		fl_set_object_lcol(dialog_->start, FL_INACTIVE);
	} else if (obj == dialog_->ignore) {
		controller().check();
	} else if (obj == dialog_->accept) {
		controller().ignoreAll();
	} else if (obj == dialog_->insert) {
		controller().insert();
	} else if (obj == dialog_->done) {
		controller().quit();
	} else if (obj == dialog_->options) {
		controller().options();
	} else if (obj == dialog_->browser) {
		int sel = fl_get_browser(dialog_->browser);
		if (clickline_==sel) {
			string tmp = fl_get_input(dialog_->input);
			controller().replace(tmp);
		}
		clickline_ = sel;
		string tmp = fl_get_browser_line(dialog_->browser, clickline_);
		fl_set_input(dialog_->input, tmp.c_str());
	}

	return ButtonPolicy::SMI_VALID;
}

void FormSpellchecker::partialUpdate(int id)
{
	// set suggestions
	if (id==0) {
		// set progress bar (always)
		fl_set_slider_value(dialog_->slider,
				    controller().getProgress());
	} else if (id==1) {
		string w = controller().getWord();
		fl_set_input(dialog_->input, w.c_str());
		fl_set_object_label(dialog_->text, w.c_str());
		fl_clear_browser(dialog_->browser);
		while (!(w = controller().getSuggestion()).empty() ) {
			fl_add_browser_line(dialog_->browser, w.c_str());
		}
	} else if (id==2) {
		fl_show_messages(controller().getMessage().c_str());
	}
}

		
void FormSpellchecker::showMessage(const char * msg)
{
	fl_show_message(msg, "", "");
}

// note there is a button accept in session
// it is not clear whether this is ingoreall or replaceall


