/**
 * \file GuiInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 * \author Jürgen Spitzmüller
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
{ "unknown", "shortcut", "shortcuts", "lyxrc", "package", "textclass", "menu", "icon", "buffer", "vcs", "lyxinfo", "" };

char const * info_types_gui[] =
{ N_("Unknown"), N_("Last Assigned Keyboard Shortcut"), N_("All Keyboard Shortcuts"),
  N_("LyX Preferences Entry"),  N_("LaTeX Package Availability"), N_("LaTeX Class Availability"),
  N_("LyX Menu Location"), N_("LyX Toolbar Icon"), N_("Document Information"),
  N_("Version Control Information"), N_("LyX Application Information"), ""};

char const * info_name_gui[] =
{ N_("Not Applicable"), N_("LyX Function"), N_("LyX Function"), N_("Preferences Key"),  N_("Package Name"),
  N_("Class Name"), N_("LyX Function"), N_("LyX Function"), N_("Not Applicable"), N_("Not Applicable"),
  N_("Not Applicable"), ""};

char const * info_tooltip[] =
{ N_("Please select a valid type above"),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the most recently assigned keyboard shortcut for this function"),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output lists all possible keyboard shortcuts for this function"),
  N_("Enter a LyX preferences key such as 'bind_file'. See the proposed list for available entries. "
     "The output is the current setting of this preference."),
  N_("Enter a LaTeX package name such as 'hyperref' (extension is optional). "
     "The output will be 'Yes' (package available) or 'No' (package unavailable)."),
  N_("Enter a LaTeX class name such as 'article' (extension is optional). "
     "The output will be 'Yes' (class available) or 'No' (class unavailable)."),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the path to the function in the menu (using the current localization)."),
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the toolbar icon for this function (using the active icon theme)."),
  N_("Please select a valid type above"),
  N_("Please select a valid type above"),
  N_("Please select a valid type above"),
  ""};


GuiInfo::GuiInfo(QWidget * parent) : InsetParamsWidget(parent)
{
	setupUi(this);

	typeCO->blockSignals(true);
	for (int n = 0; info_types[n][0]; ++n)
		typeCO->addItem(qt_(info_types_gui[n]), info_types[n]);
	typeCO->blockSignals(false);

	connect(typeCO, SIGNAL(currentIndexChanged(int)), this, SLOT(updateArguments(int)));
	connect(nameLE, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
	connect(infoLW, SIGNAL(currentTextChanged(QString)), this, SIGNAL(changed()));
}


void GuiInfo::paramsToDialog(Inset const * inset)
{
	InsetInfo const * ii = static_cast<InsetInfo const *>(inset);
	inset_ = const_cast<Inset*>(inset);
	QString const type = toqstr(ii->infoType());
	QString const name = toqstr(ii->infoName());
	typeCO->blockSignals(true);
	nameLE->blockSignals(true);
	nameLE->clear();
	int const i = typeCO->findData(type);
	typeCO->setCurrentIndex(i);
	updateArguments(i);
	int argindex = -1;
	int customindex = 0;
	for (int i = 0 ; i < infoLW->count() ; ++i) {
		if (infoLW->item(i)->data(Qt::UserRole).toString() == name)
			argindex = i;
		else if (infoLW->item(i)->data(Qt::UserRole).toString() == "custom")
			customindex = i;
	}
	if (argindex != -1)
		infoLW->setCurrentRow(argindex);
	else {
		// Without this test, 'math-insert' (name) will replace 'math-insert '
		// in nameLE and effectively disallow the input of spaces after a LFUN.
		if (nameLE->text().trimmed() != name)
			nameLE->setText(name);
		infoLW->setCurrentRow(customindex);
	}
	typeCO->blockSignals(false);
	nameLE->blockSignals(false);
}


docstring GuiInfo::dialogToParams() const
{
	QString type =
		typeCO->itemData(typeCO->currentIndex()).toString();
	QString name = infoLW->currentItem() ?
				infoLW->currentItem()->data(Qt::UserRole).toString()
			      : QString();
	if (name == "custom")
		name = nameLE->text();
	return qstring_to_ucs4(type + ' ' + name);
}


void GuiInfo::updateArguments(int i)
{
	infoLW->clear();
	if (inset_) {
		InsetInfo const * ii = static_cast<InsetInfo const *>(inset_);
		vector<pair<string,docstring>> args = ii->getArguments(info_types[i]);
		for (auto const & p : args) {
			QListWidgetItem * li = new QListWidgetItem(toqstr(p.second));
			li->setData(Qt::UserRole, toqstr(p.first));
			if (p.first == "invalid")
				// non-selectable, disabled item!
				li->setFlags(Qt::NoItemFlags);
			if (p.first == "custom")
				li->setData(Qt::ToolTipRole, qt_("Enter a valid value below"));
			infoLW->addItem(li);
		}
	}
	if (infoLW->count() > 0)
		infoLW->setCurrentRow(0);
	changed();
}


bool GuiInfo::checkWidgets(bool readonly) const
{
	nameLE->setReadOnly(readonly);
	typeCO->setEnabled(!readonly);
	nameLA->setText(qt_(info_name_gui[typeCO->currentIndex()]) + toqstr(":"));

	QString const arg = infoLW->currentItem() ?
				infoLW->currentItem()->data(Qt::UserRole).toString()
			      : QString();

	bool const type_enabled = (arg == "custom");
	nameLA->setEnabled(type_enabled);
	nameLE->setEnabled(type_enabled);
	nameLE->setToolTip(qt_(info_tooltip[typeCO->currentIndex()]));

	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !arg.isEmpty() && (arg != "custom" || !nameLE->text().isEmpty());
}



} // namespace frontend
} // namespace lyx

#include "moc_GuiInfo.cpp"
