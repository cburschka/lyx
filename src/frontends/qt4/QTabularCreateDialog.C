/**
 * \file QTabularCreateDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QTabularCreateDialog.h"
#include "QTabularCreate.h"

#include <qpushbutton.h>
#include <qspinbox.h>
#include "emptytable.h"

namespace lyx {
namespace frontend {

QTabularCreateDialog::QTabularCreateDialog(QTabularCreate * form)
	: form_(form)
{
	setupUi(this);

	rowsSB->setValue(5);
	columnsSB->setValue(5);

	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
    connect(rowsSB, SIGNAL(valueChanged(int)),
		this, SLOT( rowsChanged(int)));
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

} // namespace frontend
} // namespace lyx

#include "QTabularCreateDialog_moc.cpp"
