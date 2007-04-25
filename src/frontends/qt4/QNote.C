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
#include "Qt2BC.h"

#include "controllers/ControlNote.h"

#include "insets/InsetNote.h"

#include <QCloseEvent>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QNoteDialog
//
/////////////////////////////////////////////////////////////////////

QNoteDialog::QNoteDialog(QNote * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));

	connect(noteRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(greyedoutRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(commentRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(framedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(shadedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
}


void QNoteDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QNoteDialog::change_adaptor()
{
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// QNote
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlNote, QView<QNoteDialog> > NoteBase;


QNote::QNote(Dialog & parent)
	: NoteBase(parent, _("Note Settings"))
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
	case InsetNoteParams::Framed:
		rb = dialog_->framedRB;
		break;
	case InsetNoteParams::Shaded:
		rb = dialog_->shadedRB;
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
	else if (dialog_->framedRB->isChecked())
		type = InsetNoteParams::Framed;
	else if (dialog_->shadedRB->isChecked())
		type = InsetNoteParams::Shaded;
	else
		type = InsetNoteParams::Note;

	controller().params().type = type;
}

} // namespace frontend
} // namespace lyx

#include "QNote_moc.cpp"
