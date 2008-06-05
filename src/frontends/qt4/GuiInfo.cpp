/**
 * \file GuiInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiInfo.h"

#include "qt_helpers.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"

#include "insets/InsetInfo.h"

#include "support/debug.h"


using namespace std;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////
//
// GuiInfo
//
/////////////////////////////////////////////////////////////////

GuiInfo::GuiInfo(GuiView & lv)
	: DialogView(lv, "info", qt_("Info"))
{
	setupUi(this);
}


void GuiInfo::on_closePB_clicked()
{
	hide();
}


InsetInfo * GuiInfo::inset() const
{
	return static_cast<InsetInfo *>(bufferview()->cursor().
		innerInsetOfType(INFO_CODE));
}


void GuiInfo::applyView()
{
	InsetInfo * ii = inset();
	if (!ii)
		return;
	
	// FIXME: update the inset contents

	updateLabels(bufferview()->buffer());
	bufferview()->updateMetrics();
	bufferview()->buffer().changed();
}


void GuiInfo::updateView()
{
	InsetInfo * ii = inset();
	if (!ii) {
		// FIXME: A New button to create an InsetInfo at the cursor location
		// would be nice.
		enableView(false);
		return;
	}
	//FIXME: update the controls.
}


void GuiInfo::enableView(bool enable)
{
	//FIXME: enable controls that need enabling.
}


void GuiInfo::dispatchParams()
{
}


Dialog * createGuiInfo(GuiView & lv) { return new GuiInfo(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiInfo_moc.cpp"
