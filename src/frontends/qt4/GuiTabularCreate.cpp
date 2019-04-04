/**
 * \file GuiTabularCreate.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiTabularCreate.h"

#include "EmptyTable.h"
#include "FuncRequest.h"

#include "support/debug.h"
#include "support/convert.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"
#include "support/Package.h"

#include <QDirIterator>
#include <QSpinBox>
#include <QPushButton>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

void GuiTabularCreate::getFiles()
{
	// We look for lyx files in the subdirectory dir of
	//   1) user_lyxdir
	//   2) build_lyxdir (if not empty)
	//   3) system_lyxdir
	// in this order. Files with a given sub-hierarchy will
	// only be listed once.
	// We also consider i18n subdirectories and store them separately.
	QStringList dirs;

	// The three locations to look at.
	string const user = addPath(package().user_support().absFileName(), "tabletemplates");
	string const build = addPath(package().build_support().absFileName(), "tabletemplates");
	string const system = addPath(package().system_support().absFileName(), "tabletemplates");

	dirs << toqstr(user)
	     << toqstr(build)
	     << toqstr(system);

	for (int i = 0; i < dirs.size(); ++i) {
		QString const dir = dirs.at(i);
		QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext()) {
			QString fn = QFileInfo(it.next()).fileName();
			if (!fn.endsWith(".lyx") || fn.contains("_1x"))
				continue;
			QString data = fn.left(fn.lastIndexOf(".lyx"));
			QString guiname = data;
			guiname = toqstr(translateIfPossible(qstring_to_ucs4(guiname.replace('_', ' '))));
			QString relpath = toqstr(makeRelPath(qstring_to_ucs4(fn),
							     qstring_to_ucs4(dir)));
			if (styleCO->findData(data) == -1)
				styleCO->addItem(guiname, data);
		}
	}
}

GuiTabularCreate::GuiTabularCreate(GuiView & lv)
	: GuiDialog(lv, "tabularcreate", qt_("Insert Table"))
{
	setupUi(this);

	rowsSB->setValue(5);
	columnsSB->setValue(5);
	table->setMinimumSize(100, 100);

	connect(table, SIGNAL(rowsChanged(int)),
		rowsSB, SLOT(setValue(int)));
	connect(table, SIGNAL(colsChanged(int)),
		columnsSB, SLOT(setValue(int)));
	connect(rowsSB, SIGNAL(valueChanged(int)),
		table, SLOT(setNumberRows(int)));
	connect(columnsSB, SIGNAL(valueChanged(int)),
		table, SLOT(setNumberColumns(int)));

	connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
		this, SLOT(slotButtonBox(QAbstractButton *)));

	connect(rowsSB, SIGNAL(valueChanged(int)),
		this, SLOT(rowsChanged(int)));
	connect(columnsSB, SIGNAL(valueChanged(int)),
		this, SLOT(columnsChanged(int)));

	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
	bc().setOK(buttonBox->button(QDialogButtonBox::Ok));
	bc().setApply(buttonBox->button(QDialogButtonBox::Apply));
	bc().setCancel(buttonBox->button(QDialogButtonBox::Cancel));
	bc().setValid(isValid());

	// Fill styles combo
	styleCO->addItem(qt_("Default"), toqstr("default"));
	getFiles();
}


void GuiTabularCreate::on_styleCO_activated(int i)
{
	style_ = styleCO->itemData(i).toString();
	changed();
}


void GuiTabularCreate::columnsChanged(int)
{
	changed();
}


void GuiTabularCreate::rowsChanged(int)
{
	changed();
}


void GuiTabularCreate::applyView()
{
	params_.first = ulong(rowsSB->value());
	params_.second = ulong(columnsSB->value());
}


bool GuiTabularCreate::initialiseParams(string const &)
{
	params_.first  = 5;
	params_.second = 5;
	style_ = styleCO->itemData(styleCO->currentIndex()).toString();
	changed();
	return true;
}


void GuiTabularCreate::clearParams()
{
	params_.first  = 0;
	params_.second = 0;
}


void GuiTabularCreate::dispatchParams()
{
	string sdata;
	if (style_ != "default")
		sdata = fromqstr(style_) + ' ';
	sdata += convert<string>(params().first) + ' ' + convert<string>(params().second);
	dispatch(FuncRequest(getLfun(), sdata));
}


FuncCode GuiTabularCreate::getLfun() const
{
	if (style_.isEmpty() || style_ == "default")
		return  LFUN_TABULAR_INSERT;
	
	return LFUN_TABULAR_STYLE_INSERT;
}


Dialog * createGuiTabularCreate(GuiView & lv)
{
	return new GuiTabularCreate(lv);
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiTabularCreate.cpp"
