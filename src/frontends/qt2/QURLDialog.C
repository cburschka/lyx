/**
 * \file QURLDialog.C
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

#include "QURL.h"
#include "QURLDialog.h"

#include <qpushbutton.h>
#include <qlineedit.h>
 
QURLDialog::QURLDialog(QURL * form)
	: QURLDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}


QURLDialog::~QURLDialog()
{
}


void QURLDialog::show()
{
	QURLDialogBase::show();
	urlED->setFocus();
}

 
void QURLDialog::changed_adaptor()
{
	form_->changed();
}


void QURLDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
