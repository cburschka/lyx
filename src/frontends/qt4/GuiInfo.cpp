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
{ N_("Unknown"), N_("Last Assigned Keyboard Shortcut"), N_("All Keyboard Shortcuts"),
  N_("LyX Preferences Entry"),  N_("LaTeX Package Availability"), N_("LaTeX Class Availability"),
  N_("LyX Menu Location"), N_("LyX Toolbar Icon"), N_("Document Information"), N_("LyX Application Information"), ""};

char const * info_name_gui[] =
{ N_("Not Applicable"), N_("LyX Function"), N_("LyX Function"), N_("Preferences Key"),  N_("Package Name"),
  N_("Class Name"), N_("LyX Function"), N_("LyX Function"), N_("Information Type"), N_("Information Type"), ""};

char const * info_tooltip[] =
{ N_("Please select a valid type above"),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the most recently assigned keyboard shortcut for this function"),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output lists all possible keyboard shortcuts for this function"),
  N_("Enter a LyX preferences key such as 'bind_file'. Please refer to src/LyXRC.h for available entries. "
     "The output is the current setting of this preference."),
  N_("Enter a LaTeX package name such as 'hyperref' (extension is optional). "
     "The output will be 'Yes' (package available) or 'No' (package unavailable)."),
  N_("Enter a LaTeX class name such as 'article' (extension is optional). "
     "The output will be 'Yes' (class available) or 'No' (class unavailable)."),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the path to the function in the menu (using the current localization)."),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the toolbar icon for this function (using the active icon theme)."),
  N_("Enter either 'name' (outputs the filename of the current document), 'path' (outputs the file path), or 'class' (outputs the text class)."),
  N_("Currently supported information type: 'version' (outputs the current LyX version)."),
  ""};


GuiInfo::GuiInfo(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	typeCO->blockSignals(true);
	for (int n = 0; info_types[n][0]; ++n)
		typeCO->addItem(qt_(info_types_gui[n]), info_types[n]);
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
	int const i = typeCO->findData(type);
	typeCO->setCurrentIndex(i);
	// Without this test, 'math-insert' (name) will replace 'math-insert '
	// in nameLE and effectively disallow the input of spaces after a LFUN.
	if (nameLE->text().trimmed() != name)
		nameLE->setText(name);
	typeCO->blockSignals(false);
	nameLE->blockSignals(false);
}


docstring GuiInfo::dialogToParams() const
{
	QString type =
		typeCO->itemData(typeCO->currentIndex()).toString();
	QString const name = nameLE->text();
	return qstring_to_ucs4(type + ' ' + name);
}


bool GuiInfo::checkWidgets(bool readonly) const
{
	nameLE->setReadOnly(readonly);
	typeCO->setEnabled(!readonly);
	nameLA->setText(qt_(info_name_gui[typeCO->currentIndex()]) + toqstr(":"));
	bool const type_enabled =
		typeCO->itemData(typeCO->currentIndex()).toString() != "unknown";
	nameLA->setEnabled(type_enabled);
	nameLE->setEnabled(type_enabled);
	nameLE->setToolTip(qt_(info_tooltip[typeCO->currentIndex()]));

	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !nameLE->text().isEmpty();
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiInfo.cpp"
