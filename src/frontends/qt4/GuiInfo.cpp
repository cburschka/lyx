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
#include "Language.h"

#include "GuiApplication.h"
#include "GuiView.h"

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
{ "date",
  "moddate",
  "fixdate",
  "time",
  "modtime",
  "fixtime",
  "buffer",
  "vcs",
  "package",
  "textclass",
  "shortcut",
  "shortcuts",
  "menu",
  "l7n",
  "icon",
  "lyxrc",
  "lyxinfo",
  ""
};

// GUI names (in combo)
char const * info_types_gui[] =
{ N_("Date (current)"),// date
  N_("Date (last modified)"),// moddate
  N_("Date (fix)"),// fixdate
  N_("Time (current)"),// time
  N_("Time (last modified)"),// modtime
  N_("Time (fix)"),// fixtime
  N_("Document Information"),// buffer
  N_("Version Control Information"),// vcs
  N_("LaTeX Package Availability"),// package
  N_("LaTeX Class Availability"),// textclass
  N_("Last Assigned Keyboard Shortcut"),// shortcut
  N_("All Keyboard Shortcuts"),// shortcuts
  N_("LyX Menu Location"),// menu
  N_("Localized GUI String"),// l7n
  N_("LyX Toolbar Icon"),// icon
  N_("LyX Preferences Entry"),// lyxrc
  N_("LyX Application Information"),// lyxinfo
  ""
};

// Line edit label
char const * info_name_gui[] =
{ N_("Custom Format"),// date
  N_("Custom Format"),// moddate
  N_("Custom Format"),// fixdate
  N_("Custom Format"),// time
  N_("Custom Format"),// modtime
  N_("Custom Format"),// fixtime
  N_("Not Applicable"),// buffer
  N_("Not Applicable"),// vcs
  N_("Package Name"),// package
  N_("Class Name"),// textclass
  N_("LyX Function"),// shortcut
  N_("LyX Function"),// shortcuts
  N_("LyX Function"),// menu
  N_("English String"),// l7n
  N_("LyX Function"),// icon
  N_("Preferences Key"),// lyxrc
  N_("Not Applicable"),// lyxinfo
  N_("Not Applicable"),// unknown
  ""
};

// Line edit tooltip
char const * info_tooltip[] =
{ N_("Enter date format specification, using the following placeholders:\n"
     "* d: day as number without a leading zero\n"
     "* dd: day as number with a leading zero\n"
     "* ddd: abbreviated localized day name\n"
     "* dddd: long localized day name\n"
     "* M: month as number without a leading zero\n"
     "* MM: month as number with a leading zero\n"
     "* MMM: abbreviated localized month name\n"
     "* MMMM: long localized month name\n"
     "* yy: year as two digit number\n"
     "* yyyy: year as four digit number"),// date
  N_("Enter date format specification, using the following placeholders:\n"
     "* d: day as number without a leading zero\n"
     "* dd: day as number with a leading zero\n"
     "* ddd: abbreviated localized day name\n"
     "* dddd: long localized day name\n"
     "* M: month as number without a leading zero\n"
     "* MM: month as number with a leading zero\n"
     "* MMM: abbreviated localized month name\n"
     "* MMMM: long localized month name\n"
     "* yy: year as two digit number\n"
     "* yyyy: year as four digit number"),// moddate
  N_("Enter date format specification, using the following placeholders:\n"
     "* d: day as number without a leading zero\n"
     "* dd: day as number with a leading zero\n"
     "* ddd: abbreviated localized day name\n"
     "* dddd: long localized day name\n"
     "* M: month as number without a leading zero\n"
     "* MM: month as number with a leading zero\n"
     "* MMM: abbreviated localized month name\n"
     "* MMMM: long localized month name\n"
     "* yy: year as two digit number\n"
     "* yyyy: year as four digit number"),// fixdate
  N_("Enter time format specification, using the following placeholders:\n"
     "* h: the hour without a leading zero (1-12 in AM/PM)\n"
     "* hh: the hour with a leading zero (01-12 in AM/PM)\n"
     "* H: the hour without a leading zero (0-23 in AM/PM)\n"
     "* HH: the hour with a leading zero (00-23 in AM/PM)\n"
     "* m: the minute without a leading zero\n"
     "* mm: the minute with a leading zero\n"
     "* s: the second without a leading zero\n"
     "* ss: the second with a leading zero\n"
     "* z: the milliseconds without leading zeroes\n"
     "* zzz: the milliseconds with leading zeroes\n"
     "* AP or A: use AM/PM display ('AM'/'PM')\n"
     "* ap or a: use am/pm display ('am'/'pm')\n"
     "* t: the timezone (e.g. CEST)"),// time
  N_("Enter time format specification, using the following placeholders:\n"
     "* h: the hour without a leading zero (1-12 in AM/PM)\n"
     "* hh: the hour with a leading zero (01-12 in AM/PM)\n"
     "* H: the hour without a leading zero (0-23 in AM/PM)\n"
     "* HH: the hour with a leading zero (00-23 in AM/PM)\n"
     "* m: the minute without a leading zero\n"
     "* mm: the minute with a leading zero\n"
     "* s: the second without a leading zero\n"
     "* ss: the second with a leading zero\n"
     "* z: the milliseconds without leading zeroes\n"
     "* zzz: the milliseconds with leading zeroes\n"
     "* AP or A: use AM/PM display ('AM'/'PM')\n"
     "* ap or a: use am/pm display ('am'/'pm')\n"
     "* t: the timezone (e.g. CEST)"),// modtime
  N_("Enter time format specification, using the following placeholders:\n"
     "* h: the hour without a leading zero (1-12 in AM/PM)\n"
     "* hh: the hour with a leading zero (01-12 in AM/PM)\n"
     "* H: the hour without a leading zero (0-23 in AM/PM)\n"
     "* HH: the hour with a leading zero (00-23 in AM/PM)\n"
     "* m: the minute without a leading zero\n"
     "* mm: the minute with a leading zero\n"
     "* s: the second without a leading zero\n"
     "* ss: the second with a leading zero\n"
     "* z: the milliseconds without leading zeroes\n"
     "* zzz: the milliseconds with leading zeroes\n"
     "* AP or A: use AM/PM display ('AM'/'PM')\n"
     "* ap or a: use am/pm display ('am'/'pm')\n"
     "* t: the timezone (e.g. CEST)"),// fixtime
  N_("Please select a valid type above"),// buffer
  N_("Please select a valid type above"),// vcs
  N_("Enter a LaTeX package name such as 'hyperref' (extension is optional). "
     "The output will be 'Yes' (package available) or 'No' (package unavailable)."),// package
  N_("Enter a LaTeX class name such as 'article' (extension is optional). "
     "The output will be 'Yes' (class available) or 'No' (class unavailable)."),// textclass
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the most recently assigned keyboard shortcut for this function"),// shortcut
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output lists all possible keyboard shortcuts for this function"),// shortcuts
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the path to the function in the menu (using the current localization)."),// menu
  N_("Enter a localizable English string from the LyX User Interface, including accelerator markup ('&' or '|') and trailing colons. "
     "The output is the localized string (using the current localization); trailing colons and accelerator markup are stripped."),// l7n
  N_("Enter a function name such as 'math-insert \\alpha'. Please refer to Help > LyX Functions for a comprehensive list of functions. "
     "The output is the toolbar icon for this function (using the active icon theme)."),// icon
  N_("Enter a LyX preferences key such as 'bind_file'. See the proposed list for available entries. "
     "The output is the current setting of this preference."),// lyxrc
  N_("Please select a valid type above"),// lyxinfo
  N_("Please select a valid type above"),// unknown
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
	connect(fixDateLE, SIGNAL(textChanged(QString)), this, SIGNAL(changed()));
	connect(infoLW, SIGNAL(currentTextChanged(QString)), this, SIGNAL(changed()));
}


void GuiInfo::paramsToDialog(Inset const * inset)
{
	InsetInfo const * ii = static_cast<InsetInfo const *>(inset);
	params_ = ii->params();
	QString const type = toqstr(params_.infoType());
	QString name = toqstr(params_.name);
	QString fixdate;
	if (type == "fixdate" || type == "fixtime") {
		fixdate = name.section('@', 1, 1);
		name = name.section('@', 0, 0);
		if (!fixdate.isEmpty())
			fixDateLE->setText(fixdate);
		else {
			if (type == "fixdate")
				fixDateLE->setText(QDate::currentDate().toString(Qt::ISODate));
			else
				fixDateLE->setText(QTime::currentTime().toString(Qt::ISODate));
		}
	}
	typeCO->blockSignals(true);
	nameLE->blockSignals(true);
	nameLE->clear();
	// The "unknown" item is only in the combo if we open
	// the dialog from an unknown info. The user should not
	// change a valid info to an unknown one.
	int i = typeCO->findData("unknown");
	if (i == -1 && type == "unknown")
		typeCO->addItem(qt_("Unknown"), toqstr("unknown"));
	if (i != -1 && type != "unknown")
		typeCO->removeItem(i);
	i = typeCO->findData(type);
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
	if (type == "fixdate" || type == "fixtime") {
		QString fd = fixDateLE->text();
		if (fd.isEmpty())
			fd = (type == "fixdate") ?
					QDate::currentDate().toString(Qt::ISODate)
				      : QTime::currentTime().toString(Qt::ISODate);
		name += "@" + fd;
	}
	return qstring_to_ucs4(type + ' ' + name);
}


bool GuiInfo::initialiseParams(std::string const & sdata)
{
	Language const * lang = languages.getLanguage(sdata);
	if (!lang)
		return false;
	params_.lang = lang;
	updateArguments(typeCO->currentIndex());
	return true;
}


void GuiInfo::updateArguments(int i)
{
	if (i == -1)
		i = 0;

	infoLW->clear();
	BufferView const * bv = guiApp->currentView()->currentBufferView();
	vector<pair<string,docstring>> args = params_.getArguments(const_cast<Buffer *>(&bv->buffer()),
								   info_types[i]);
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

	string const typestr = info_types[typeCO->currentIndex()];
	bool const fixdate_enabled = (typestr == "fixdate" || typestr == "fixtime");
	fixDateLE->setVisible(fixdate_enabled);
	fixDateLA->setVisible(fixdate_enabled);
	if (typestr == "fixdate") {
		fixDateLE->setToolTip(qt_("Here you can enter a fix date (in ISO format: YYYY-MM-DD)"));
		fixDateLA->setText(qt_("&Fix Date:"));
	}
	else if (typestr == "fixtime") {
		fixDateLE->setToolTip(qt_("Here you can enter a fix time (in ISO format: hh:mm:ss)"));
		fixDateLA->setText(qt_("&Fix Time:"));
	}

	if (!InsetParamsWidget::checkWidgets())
		return false;
	return !arg.isEmpty() && (arg != "custom" || !nameLE->text().isEmpty());
}



} // namespace frontend
} // namespace lyx

#include "moc_GuiInfo.cpp"
