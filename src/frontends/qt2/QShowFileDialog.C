/**
 * \file QShowFileDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "QShowFile.h"
#include "QShowFileDialog.h"

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
