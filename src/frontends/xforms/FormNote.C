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
	: base_class(parent, _("LyX: Note Settings"))
{}


void FormNote::build()
{
	dialog_.reset(build_note(this));


	tooltips().init(dialog_->radio_note,
			_("LyX internal only"));
	tooltips().init(dialog_->radio_comment,
			_("Export to LaTeX/Docbook but don't print"));
	tooltips().init(dialog_->radio_greyedout,
			_("Print as grey text"));

	bcview().setOK(dialog_->button_ok);
	bcview().setCancel(dialog_->button_cancel);
}


void FormNote::update()
{
	FL_OBJECT * rb = 0;

	switch (controller().params().type) {
	case InsetNoteParams::Note:
		rb = dialog_->radio_note;
		break;
	case InsetNoteParams::Comment:
		rb = dialog_->radio_comment;
		break;
	case InsetNoteParams::Greyedout:
		rb = dialog_->radio_greyedout;
		break;
	}

	fl_set_button(rb, 1);
}


void FormNote::apply()
{
	InsetNoteParams::Type type;

	if (fl_get_button(dialog_->radio_greyedout))
		type = InsetNoteParams::Greyedout;
	else if (fl_get_button(dialog_->radio_comment))
		type = InsetNoteParams::Comment;
	else
		type = InsetNoteParams::Note;

	controller().params().type = type;
}
