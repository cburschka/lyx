/**
 * \file GuiFloat.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiFloat.h"

#include "FloatPlacement.h"
#include "FuncRequest.h"

#include "insets/InsetFloat.h"

#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

GuiFloat::GuiFloat(GuiView & lv)
	: GuiDialog(lv, "float", qt_("Float Settings"))
{
	setupUi(this);
	
	connect(restorePB, SIGNAL(clicked()), this, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), this, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	// enable span columns checkbox
	floatFP->useWide();
	// enable sideways checkbox
	floatFP->useSideways();

	connect(floatFP, SIGNAL(changed()), this, SLOT(change_adaptor()));

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);

	bc().setCancel(closePB);
	bc().setApply(applyPB);
	bc().setOK(okPB);
	bc().setRestore(restorePB);

	bc().addReadOnly(floatFP);
}


void GuiFloat::change_adaptor()
{
	changed();
}


void GuiFloat::updateContents()
{
	floatFP->set(params_);
}


void GuiFloat::applyView()
{
	params_.placement = floatFP->get(params_.wide, params_.sideways);
}


bool GuiFloat::initialiseParams(string const & data)
{
	InsetFloat::string2params(data, params_);
	return true;
}


void GuiFloat::clearParams()
{
	params_ = InsetFloatParams();
}


void GuiFloat::dispatchParams()
{
	dispatch(FuncRequest(getLfun(), InsetFloat::params2string(params_)));
}


Dialog * createGuiFloat(GuiView & lv) { return new GuiFloat(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiFloat.cpp"
