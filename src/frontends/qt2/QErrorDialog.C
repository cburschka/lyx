/**
 * \file QErrorDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include "QErrorDialog.h"
#include "Dialogs.h"
#include "QError.h"

#include <qwidget.h>
#include <qpushbutton.h>

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
