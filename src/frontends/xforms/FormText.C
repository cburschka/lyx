/**
 * \file FormText.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "xformsBC.h"
#include "ControlCommand.h"
#include "FormText.h"
#include "forms/form_text.h"
#include "forms_gettext.h"

#include "support/lstrings.h"

#include FORMS_H_LOCATION

typedef FormController<ControlCommand, FormView<FD_text> > base_class;

FormText::FormText(Dialog & parent, string const & title, string const & label)
	: base_class(parent, title), label_(label)
{}


void FormText::build()
{
	dialog_.reset(build_text(this));

	fl_set_input_return(dialog_->input_text, FL_RETURN_CHANGED);
	setPrehandler(dialog_->input_text);

	fl_set_object_label(dialog_->input_text, idex(label_).c_str());
	fl_set_button_shortcut(dialog_->input_text, scex(label_).c_str(), 1);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->input_text);
}


void FormText::update()
{
	string const contents = trim(controller().params().getContents());
	fl_set_input(dialog_->input_text, contents.c_str());

	bc().valid(!contents.empty());
}


void FormText::apply()
{
	controller().params().setContents(fl_get_input(dialog_->input_text));
}
