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

#include "debug.h"
#include "ControlNote.h"
#include "insets/insetnote.h"

#include <qradiobutton.h>
#include <qpushbutton.h>

#include "QNoteDialog.h"
#include "QNote.h"
#include "Qt2BC.h"

using std::string;


typedef QController<ControlNote, QView<QNoteDialog> > base_class;


QNote::QNote(Dialog & parent)
	: base_class(parent, _("LyX: Note Settings"))
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
	string type(controller().params().type);

	if (type == "Note")
		rb = dialog_->noteRB;
	else if (type == "Comment")
		rb = dialog_->commentRB;
	else if (type == "Greyedout")
		rb = dialog_->greyedoutRB;

	rb->setChecked(true);
}


void QNote::apply()
{
	string type;

	if (dialog_->greyedoutRB->isChecked())
		type = "Greyedout";
	else if (dialog_->commentRB->isChecked())
		type = "Comment";
	else
		type = "Note";

	controller().params().type = type;
}
