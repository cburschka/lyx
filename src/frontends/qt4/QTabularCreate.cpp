/**
 * \file QTabularCreate.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QTabularCreate.h"
#include "Qt2BC.h"
#include "EmptyTable.h"

#include "controllers/ControlTabularCreate.h"

#include <QSpinBox>
#include <QPushButton>

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QTabularCreateDialog
//
/////////////////////////////////////////////////////////////////////

QTabularCreateDialog::QTabularCreateDialog(QTabularCreate * form)
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


void QTabularCreateDialog::columnsChanged(int)
{
	form_->changed();
}


void QTabularCreateDialog::rowsChanged(int)
{
	form_->changed();
}


/////////////////////////////////////////////////////////////////////
//
// QTabularCreate
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlTabularCreate, QView<QTabularCreateDialog> >
	TabularCreateBase;


QTabularCreate::QTabularCreate(Dialog & parent)
	: TabularCreateBase(parent, _("Insert Table"))
{
}


void QTabularCreate::build_dialog()
{
	dialog_.reset(new QTabularCreateDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setCancel(dialog_->closePB);
}


void QTabularCreate::apply()
{
	controller().params().first = dialog_->rowsSB->value();
	controller().params().second = dialog_->columnsSB->value();
}

} // namespace frontend
} // namespace lyx

#include "QTabularCreate_moc.cpp"
