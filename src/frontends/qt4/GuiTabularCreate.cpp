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

#include "support/convert.h"

#include <QSpinBox>
#include <QPushButton>

using namespace std;

namespace lyx {
namespace frontend {

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

	connect(okPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(rowsSB, SIGNAL(valueChanged(int)),
		this, SLOT(rowsChanged(int)));
	connect(columnsSB, SIGNAL(valueChanged(int)),
		this, SLOT(columnsChanged(int)));

	bc().setPolicy(ButtonPolicy::IgnorantPolicy);
	bc().setOK(okPB);
	bc().setCancel(closePB);
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
	params_.first = rowsSB->value();
	params_.second = columnsSB->value();
}


bool GuiTabularCreate::initialiseParams(string const &)
{
	params_.first  = 5;
	params_.second = 5;
	return true;
}


void GuiTabularCreate::clearParams()
{
	params_.first  = 0;
	params_.second = 0;
}


void GuiTabularCreate::dispatchParams()
{
	string const data = convert<string>(params().first) + ' ' + convert<string>(params().second);
	dispatch(FuncRequest(getLfun(), data));
}


Dialog * createGuiTabularCreate(GuiView & lv)
{
	return new GuiTabularCreate(lv);
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiTabularCreate.cpp"
