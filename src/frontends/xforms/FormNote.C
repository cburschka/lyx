/**
 * \file FormNote.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS
 */


#include <config.h>

#include "xformsBC.h"
#include "ControlNote.h"
#include "FormNote.h"
#include "forms/form_note.h"
#include "Tooltips.h"
#include "lyx_forms.h"
#include "insets/insetnote.h"
#include "debug.h"

typedef FormController<ControlNote, FormView<FD_note> > base_class;

FormNote::FormNote(Dialog & parent)
	: base_class(parent, _("Note"))
{}


string const FormNote::predefineds() const
{
	return _("Note|Comment|Greyedout");
}


void FormNote::build()
{
	dialog_.reset(build_note(this));

	fl_addto_choice(dialog_->choice_type, predefineds().c_str());
	string str = _("Note: LyX internal only\n"
		       "Comment: Export to LaTeX but don't print\n"
		       "Greyedout: Print as grey text");
	tooltips().init(dialog_->choice_type, str);

	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_cancel);
}


void FormNote::update()
{
	string type(controller().params().type);
	fl_set_choice_text(dialog_->choice_type, type.c_str());
}


void FormNote::apply()
{
	controller().params().type = fl_get_choice_text(dialog_->choice_type);
}

