/**
 * \file QTabularCreateDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>
#include <gettext.h>

#include "QTabularCreateDialog.h"
#include "QTabularCreate.h"
 
#include "support/lstrings.h"

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

 
void QTabularCreateDialog::columnsChanged(int nr_cols)
{
	form_->changed();
}

 
void QTabularCreateDialog::rowsChanged(int nr_rows)
{
	form_->changed();
}
