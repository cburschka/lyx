/**
 * \file QErrorDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>
 
#ifdef __GNUG__
#pragma implementation
#endif

#include "QError.h"
 
#include <qwidget.h>
#include <qpushbutton.h>

#include "QErrorDialog.h"

QErrorDialog::QErrorDialog(QError * form)
	: QErrorDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QErrorDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
