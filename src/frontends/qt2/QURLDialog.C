/**
 * \file QURLDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
 
#include <qpushbutton.h>
 
#include "QURLDialog.h"
#include "Dialogs.h"
#include "QURL.h"

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

 
void QURLDialog::changed_adaptor()
{
	form_->changed();
}


void QURLDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
