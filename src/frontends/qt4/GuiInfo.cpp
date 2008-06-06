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
#include "support/lstrings.h"


using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////
//
// GuiInfo
//
/////////////////////////////////////////////////////////////////

char const * info_types[] =
{ "unknown", "shortcut", "shortcuts", "lyxrc", "package", "textclass", "menu", "icon", "buffer", "" };

char const * info_types_gui[] =
{ N_("unknown"), N_("shortcut"), N_("shortcuts"), N_("lyxrc"), N_("package"), N_("textclass"),
  N_("menu"), N_("icon"), N_("buffer"), ""};



GuiInfo::GuiInfo(GuiView & lv)
	: DialogView(lv, "info", qt_("Info"))
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotApply()));

	connect(typeCO, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(nameLE, SIGNAL(textChanged(QString)), this, SLOT(change_adaptor()));

	for (int n = 0; info_types[n][0]; ++n)
		typeCO->addItem(qt_(info_types_gui[n]));
}


void GuiInfo::on_cancelPB_clicked()
{
	hide();
}


void GuiInfo::applyView()
{
	InsetInfo * ii = static_cast<InsetInfo *>(inset(INFO_CODE));
	if (!ii)
		return;
	
	// FIXME: update the inset contents

	updateLabels(bufferview()->buffer());
	bufferview()->updateMetrics();
	bufferview()->buffer().changed();

	hide();
}


void GuiInfo::updateView()
{
	InsetInfo * ii = static_cast<InsetInfo *>(inset(INFO_CODE));
	if (!ii) {
		typeCO->setCurrentIndex(0);
		nameLE->clear();
		// FIXME: A New button to create an InsetInfo at the cursor location
		// would be nice.
		enableView(false);
		return;
	}

	int type = findToken(info_types, ii->infoType());
	typeCO->setCurrentIndex(type >= 0 ? type : 0);
	nameLE->setText(toqstr(ii->infoName()));
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
