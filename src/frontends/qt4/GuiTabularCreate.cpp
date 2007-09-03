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

#include <QCloseEvent>
#include <QSpinBox>
#include <QPushButton>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// GuiTabularCreateDialog
//
/////////////////////////////////////////////////////////////////////

GuiTabularCreateDialog::GuiTabularCreateDialog(GuiTabularCreate * form)
	: form_(form)
{
	setupUi(this);

	rowsSB->setValue(5);
	columnsSB->setValue(5);

	connect(okPB, SIGNAL(clicked()), form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));

	connect(rowsSB, SIGNAL(valueChanged(int)),
		this, SLOT(rowsChanged(int)));
	connect(columnsSB, SIGNAL(valueChanged(int)),
		this, SLOT(columnsChanged(int)));
}


void GuiTabularCreateDialog::columnsChanged(int)
{
	form_->changed();
}


void GuiTabularCreateDialog::rowsChanged(int)
{
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// GuiTabularCreate
//
/////////////////////////////////////////////////////////////////////


GuiTabularCreate::GuiTabularCreate(GuiDialog & parent)
	: GuiView<GuiTabularCreateDialog>(parent, _("Insert Table"))
{
}


void GuiTabularCreate::build_dialog()
{
	dialog_.reset(new GuiTabularCreateDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
}


void GuiTabularCreate::applyView()
{
	controller().params().first = dialog_->rowsSB->value();
	controller().params().second = dialog_->columnsSB->value();
}

} // namespace frontend
} // namespace lyx

#include "GuiTabularCreate_moc.cpp"
