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

#include "ControlNote.h"
#include "insets/InsetNote.h"

#include <QCloseEvent>

namespace lyx {
namespace frontend {

GuiNoteDialog::GuiNoteDialog(LyXView & lv)
	: GuiDialog(lv, "note")
{
	setupUi(this);
	setController(new ControlNote(*this));
	setViewTitle(_("Note Settings"));

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(noteRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(greyedoutRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(commentRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(framedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(shadedRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
}


ControlNote & GuiNoteDialog::controller() const
{
	return static_cast<ControlNote &>(GuiDialog::controller());
}


void GuiNoteDialog::closeEvent(QCloseEvent * e)
{
	slotWMHide();
	e->accept();
}


void GuiNoteDialog::change_adaptor()
{
	changed();
}


void GuiNoteDialog::update_contents()
{
	QRadioButton * rb = 0;

	switch (controller().params().type) {
	case InsetNoteParams::Note:
		rb = noteRB;
		break;
	case InsetNoteParams::Comment:
		rb = commentRB;
		break;
	case InsetNoteParams::Greyedout:
		rb = greyedoutRB;
		break;
	case InsetNoteParams::Framed:
		rb = framedRB;
		break;
	case InsetNoteParams::Shaded:
		rb = shadedRB;
		break;
	}

	rb->setChecked(true);
}


void GuiNoteDialog::applyView()
{
	InsetNoteParams::Type type;

	if (greyedoutRB->isChecked())
		type = InsetNoteParams::Greyedout;
	else if (commentRB->isChecked())
		type = InsetNoteParams::Comment;
	else if (framedRB->isChecked())
		type = InsetNoteParams::Framed;
	else if (shadedRB->isChecked())
		type = InsetNoteParams::Shaded;
	else
		type = InsetNoteParams::Note;

	controller().params().type = type;
}

} // namespace frontend
} // namespace lyx

#include "GuiNote_moc.cpp"
