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

#include "insets/InsetInfo.h"

#include "support/debug.h"
#include "support/gettext.h"
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
{ "unknown", "shortcut", "shortcuts", "lyxrc", "package", "textclass", "menu", "icon", "buffer", "lyxinfo", "" };

char const * info_types_gui[] =
{ N_("unknown"), N_("shortcut"), N_("shortcuts"), N_("lyxrc"), N_("package"), N_("textclass"),
  N_("menu"), N_("icon"), N_("buffer"), N_("lyxinfo"), ""};


GuiInfo::GuiInfo(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	typeCO->blockSignals(true);
	for (int n = 0; info_types[n][0]; ++n)
		typeCO->addItem(qt_(info_types_gui[n]));
	typeCO->blockSignals(false);

	connect(typeCO, SIGNAL(currentIndexChanged(int)), this, SIGNAL(changed()));
	connect(nameLE, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
}


void GuiInfo::paramsToDialog(Inset const * inset)
{
	InsetInfo const * ii = static_cast<InsetInfo const *>(inset);
	QString const type = toqstr(ii->infoType());
	QString const name = toqstr(ii->infoName());
	typeCO->blockSignals(true);
	nameLE->blockSignals(true);
	int type_index = findToken(info_types, fromqstr(type));
	typeCO->setCurrentIndex(type_index >= 0 ? type_index : 0);
	// Without this test, 'math-insert' (name) will replace 'math-insert '
	// in nameLE and effectively disallow the input of spaces after a LFUN.
	if (nameLE->text().trimmed() != name)
		nameLE->setText(name);
	typeCO->blockSignals(false);
	nameLE->blockSignals(false);
}


docstring GuiInfo::dialogToParams() const
{
	int type_index = typeCO->currentIndex();
	QString type;
	if (type_index != -1)
		type = info_types[type_index];
	QString const name = nameLE->text();
	return qstring_to_ucs4(type + ' ' + name);
}


bool GuiInfo::checkWidgets(bool readonly) const
{
	nameLE->setReadOnly(readonly);
	typeCO->setEnabled(!readonly);
	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !nameLE->text().isEmpty();
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiInfo.cpp"
