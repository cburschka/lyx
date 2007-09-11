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

#include "ControlTabularCreate.h"
#include "EmptyTable.h"

#include <QCloseEvent>
#include <QSpinBox>
#include <QPushButton>


namespace lyx {
namespace frontend {

GuiTabularCreateDialog::GuiTabularCreateDialog(LyXView & lv)
	: GuiDialog(lv, "tabularcreate")
{
	setupUi(this);
	setViewTitle(_("Insert Table"));
	setController(new ControlTabularCreate(*this));

	rowsSB->setValue(5);
	columnsSB->setValue(5);

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


ControlTabularCreate & GuiTabularCreateDialog::controller()
{
	return static_cast<ControlTabularCreate &>(GuiDialog::controller());
}


void GuiTabularCreateDialog::columnsChanged(int)
{
	changed();
}


void GuiTabularCreateDialog::rowsChanged(int)
{
	changed();
}


void GuiTabularCreateDialog::applyView()
{
	controller().params().first = rowsSB->value();
	controller().params().second = columnsSB->value();
}

} // namespace frontend
} // namespace lyx

#include "GuiTabularCreate_moc.cpp"
