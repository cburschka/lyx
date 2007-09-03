/**
 * \file GuiNote.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiNote.h"

#include "insets/InsetNote.h"

#include <QCloseEvent>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiNoteDialog
//
/////////////////////////////////////////////////////////////////////

GuiNoteDialog::GuiNoteDialog(GuiNote * form)
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


void GuiNoteDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void GuiNoteDialog::change_adaptor()
{
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// GuiNote
//
/////////////////////////////////////////////////////////////////////


GuiNote::GuiNote(GuiDialog & parent)
	: GuiView<GuiNoteDialog>(parent, _("Note Settings"))
{}


void GuiNote::build_dialog()
{
	dialog_.reset(new GuiNoteDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
}


void GuiNote::update_contents()
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


void GuiNote::applyView()
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

#include "GuiNote_moc.cpp"
