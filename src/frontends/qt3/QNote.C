/**
 * \file QNote.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QNote.h"
#include "QNoteDialog.h"
#include "Qt2BC.h"

#include "controllers/ControlNote.h"

#include "insets/insetnote.h"

#include <qradiobutton.h>
#include <qpushbutton.h>


using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlNote, QView<QNoteDialog> > base_class;


QNote::QNote(Dialog & parent)
	: base_class(parent, _("Note Settings"))
{}


void QNote::build_dialog()
{
	dialog_.reset(new QNoteDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
}


void QNote::update_contents()
{
	QRadioButton * rb = 0;

	switch (controller().params().type) {
	case InsetNoteParams::Note:
		rb = dialog_->noteRB;
		break;
	case InsetNoteParams::Comment:
		rb = dialog_->commentRB;
		break;
	case InsetNoteParams::Greyedout:
		rb = dialog_->greyedoutRB;
		break;
	}

	rb->setChecked(true);
}


void QNote::apply()
{
	InsetNoteParams::Type type;

	if (dialog_->greyedoutRB->isChecked())
		type = InsetNoteParams::Greyedout;
	else if (dialog_->commentRB->isChecked())
		type = InsetNoteParams::Comment;
	else
		type = InsetNoteParams::Note;

	controller().params().type = type;
}

} // namespace frontend
} // namespace lyx
