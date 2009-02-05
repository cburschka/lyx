/**
 * \file GuiPhantom.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiPhantom.h"
#include "FuncRequest.h"
#include "support/gettext.h"

#include "insets/InsetPhantom.h"

using namespace std;

namespace lyx {
namespace frontend {

GuiPhantom::GuiPhantom(GuiView & lv)
	: GuiDialog(lv, "phantom", qt_("Phantom Settings"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(phantomRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(hphantomRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(vphantomRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
}


void GuiPhantom::change_adaptor()
{
	changed();
}


void GuiPhantom::updateContents()
{
	switch (params_.type) {
	case InsetPhantomParams::Phantom:
		phantomRB->setChecked(true);
		break;
	case InsetPhantomParams::HPhantom:
		hphantomRB->setChecked(true);
		break;
	case InsetPhantomParams::VPhantom:
		vphantomRB->setChecked(true);
		break;
	}
}


void GuiPhantom::applyView()
{
	if (vphantomRB->isChecked())
		params_.type = InsetPhantomParams::VPhantom;
	else if (hphantomRB->isChecked())
		params_.type = InsetPhantomParams::HPhantom;
	else
		params_.type = InsetPhantomParams::Phantom;
}


bool GuiPhantom::initialiseParams(string const & data)
{
	InsetPhantom::string2params(data, params_);
	return true;
}


void GuiPhantom::clearParams()
{
	params_ = InsetPhantomParams();
}


void GuiPhantom::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetPhantom::params2string(params_)));
}


Dialog * createGuiPhantom(GuiView & lv) { return new GuiPhantom(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiPhantom.cpp"
