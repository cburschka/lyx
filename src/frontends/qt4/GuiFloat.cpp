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

#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

GuiFloat::GuiFloat(GuiView & lv)
	: InsetDialog(lv, FLOAT_CODE, LFUN_FLOAT_INSERT, "float", "Float Settings")
{
	setupUi(this);
	
	// enable span columns checkbox
	floatFP->useWide();
	// enable sideways checkbox
	floatFP->useSideways();

	connect(floatFP, SIGNAL(changed()), this, SLOT(applyView()));
}


void GuiFloat::enableView(bool enable)
{
	floatFP->setEnabled(enable);
}


void GuiFloat::paramsToDialog(Inset const * inset)
{
	floatFP->paramsToDialog(inset);
}


docstring GuiFloat::dialogToParams() const
{
	return floatFP->dialogToParams();
}

Dialog * createGuiFloat(GuiView & lv) { return new GuiFloat(lv); }

} // namespace frontend
} // namespace lyx

#include "moc_GuiFloat.cpp"
