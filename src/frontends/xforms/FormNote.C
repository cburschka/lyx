/**
 * \file FormNote.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormNote.h"
#include "ControlNote.h"
#include "forms/form_note.h"

#include "Tooltips.h"
#include "xformsBC.h"

#include "insets/insetnote.h"

#include "lyx_forms.h"


using std::string;


typedef FormController<ControlNote, FormView<FD_note> > base_class;

FormNote::FormNote(Dialog & parent)
	: base_class(parent, _("Note"))
{}


void FormNote::build()
{
	dialog_.reset(build_note(this));

	note_gui_tokens(ids_, gui_names_);

	for (string::size_type i = 0; i < gui_names_.size(); ++i) {
		fl_addto_choice(dialog_->choice_type, gui_names_[i].c_str());
	}

	string str = _("Lyx Note: LyX internal only\n"
		       "Comment: Export to LaTeX but don't print\n"
		       "Greyed Out: Print as grey text");
	tooltips().init(dialog_->choice_type, str);

	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_cancel);
}


void FormNote::update()
{
	string type(controller().params().type);
	for (string::size_type i = 0; i < gui_names_.size(); ++i) {
		if (type == ids_[i])
			fl_set_choice_text(dialog_->choice_type, gui_names_[i].c_str());
		}
}


void FormNote::apply()
{
	int i = fl_get_choice(dialog_->choice_type);
	controller().params().type = ids_[i - 1];
}
