/**
 * \file QTabularCreateDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "gettext.h"

#include "support/lstrings.h"

#include "QTabularCreate.h"
#include "QTabularCreateDialog.h"

#include <qpushbutton.h>
#include <qspinbox.h>
#include "emptytable.h"


QTabularCreateDialog::QTabularCreateDialog(QTabularCreate * form)
	: QTabularCreateDialogBase(0, 0, false, 0),
	form_(form)
{
	table->setMinimumSize(100,100);
	rowsSB->setValue(5);
	columnsSB->setValue(5);

	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}


void QTabularCreateDialog::columnsChanged(int)
{
	form_->changed();
}


void QTabularCreateDialog::rowsChanged(int)
{
	form_->changed();
}
