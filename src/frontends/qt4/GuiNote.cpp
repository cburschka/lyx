/**
 * \file GuiNote.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiNote.h"
#include "FuncRequest.h"
#include "gettext.h"

#include "insets/InsetNote.h"

#include <QCloseEvent>

using std::string;


namespace lyx {
namespace frontend {

GuiNote::GuiNote(LyXView & lv)
	: GuiDialog(lv, "note"), Controller(this)
{
	setupUi(this);
	setController(this, false);
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


void GuiNote::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiNote::change_adaptor()
{
	changed();
}


void GuiNote::updateContents()
{
	switch (params_.type) {
	case InsetNoteParams::Note:
		noteRB->setChecked(true);
		break;
	case InsetNoteParams::Comment:
		commentRB->setChecked(true);
		break;
	case InsetNoteParams::Greyedout:
		greyedoutRB->setChecked(true);
		break;
	case InsetNoteParams::Framed:
		framedRB->setChecked(true);
		break;
	case InsetNoteParams::Shaded:
		shadedRB->setChecked(true);
		break;
	}
}


void GuiNote::applyView()
{
	if (greyedoutRB->isChecked())
		params_.type = InsetNoteParams::Greyedout;
	else if (commentRB->isChecked())
		params_.type = InsetNoteParams::Comment;
	else if (framedRB->isChecked())
		params_.type = InsetNoteParams::Framed;
	else if (shadedRB->isChecked())
		params_.type = InsetNoteParams::Shaded;
	else
		params_.type = InsetNoteParams::Note;
}


bool GuiNote::initialiseParams(string const & data)
{
	InsetNoteMailer::string2params(data, params_);
	return true;
}


void GuiNote::clearParams()
{
	params_ = InsetNoteParams();
}


void GuiNote::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetNoteMailer::params2string(params_)));
}


Dialog * createGuiNote(LyXView & lv) { return new GuiNote(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiNote_moc.cpp"
