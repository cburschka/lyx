/**
 * \file QShowFileDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include "QShowFileDialog.h"
#include "Dialogs.h"
#include "QShowFile.h"

#include <qwidget.h>
#include <qpushbutton.h>

QShowFileDialog::QShowFileDialog(QShowFile * form)
	: QShowFileDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}

 
void QShowFileDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
