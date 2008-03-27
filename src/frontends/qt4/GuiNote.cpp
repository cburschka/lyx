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
#include "support/gettext.h"

#include "insets/InsetNote.h"

using namespace std;

namespace lyx {
namespace frontend {

GuiNote::GuiNote(GuiView & lv)
	: GuiDialog(lv, "note", qt_("Note Settings"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(noteRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(greyedoutRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(commentRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
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
	}
}


void GuiNote::applyView()
{
	if (greyedoutRB->isChecked())
		params_.type = InsetNoteParams::Greyedout;
	else if (commentRB->isChecked())
		params_.type = InsetNoteParams::Comment;
	else
		params_.type = InsetNoteParams::Note;
}


bool GuiNote::initialiseParams(string const & data)
{
	InsetNote::string2params(data, params_);
	return true;
}


void GuiNote::clearParams()
{
	params_ = InsetNoteParams();
}


void GuiNote::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetNote::params2string(params_)));
}


Dialog * createGuiNote(GuiView & lv) { return new GuiNote(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiNote_moc.cpp"
