/**
 * \file QERTDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
 
#include "QERT.h"
#include "QERTDialog.h"

#include <qpushbutton.h>

QERTDialog::QERTDialog(QERT * form)
	: QERTDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QERTDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
