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

	typeCO->blockSignals(true);
	for (int n = 0; info_types[n][0]; ++n)
		typeCO->addItem(qt_(info_types_gui[n]));
	typeCO->blockSignals(false);
}


void GuiInfo::on_newPB_clicked()
{
	dialogToParams();
	docstring const argument = qstring_to_ucs4(type_ + ' ' + name_);
	dispatch(FuncRequest(LFUN_INFO_INSERT, argument));
}


void GuiInfo::on_closePB_clicked()
{
	hide();
}


void GuiInfo::on_typeCO_currentIndexChanged(int)
{
	applyView();
}


void GuiInfo::on_nameLE_textChanged(QString const &)
{
	applyView();
}


void GuiInfo::applyView()
{
	InsetInfo const * ii = static_cast<InsetInfo const *>(inset(INFO_CODE));
	if (!ii) {
		return;
	}
	
	dialogToParams();
	docstring const argument = qstring_to_ucs4(type_ + ' ' + name_);
	if (!ii->validate(argument))
		return;

	dispatch(FuncRequest(LFUN_INSET_MODIFY, argument));
	// FIXME: update the inset contents
	updateLabels(bufferview()->buffer());
	BufferView * bv = const_cast<BufferView *>(bufferview());
	bv->updateMetrics();
	bv->buffer().changed();
	bv->buffer().markDirty();
}


void GuiInfo::updateView()
{
	InsetInfo const * ii = static_cast<InsetInfo const *>(inset(INFO_CODE));
	if (!ii) {
		enableView(false);
		return;
	}

	type_ = toqstr(ii->infoType());
	name_ = toqstr(ii->infoName());
	paramsToDialog();
}


void GuiInfo::paramsToDialog()
{
	typeCO->blockSignals(true);
	nameLE->blockSignals(true);
	int type = findToken(info_types, fromqstr(type_));
	typeCO->setCurrentIndex(type >= 0 ? type : 0);
	// Without this test, 'math-insert' (name_) will replace 'math-insert '
	// in nameLE and effectively disallow the input of spaces after a LFUN.
	if (nameLE->text().trimmed() != name_)
		nameLE->setText(name_);
	typeCO->blockSignals(false);
	nameLE->blockSignals(false);
}


void GuiInfo::dialogToParams()
{
	int type = typeCO->currentIndex();
	if (type != -1)
		type_ = info_types[type];
	name_ = nameLE->text();
}


void GuiInfo::enableView(bool enable)
{
	typeCO->setEnabled(enable);
	nameLE->setEnabled(enable);
	newPB->setEnabled(!enable);
}


Dialog * createGuiInfo(GuiView & lv) { return new GuiInfo(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiInfo.cpp"
