/**
 * \file QVCLogDialog.C
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

#include <vector>

#include "LyXView.h"
#include "ControlVCLog.h"

#include "QVCLog.h"
#include "QVCLogDialog.h"

#include <qpushbutton.h>
#include <qtextview.h>
 
QVCLogDialog::QVCLogDialog(QVCLog * form)
	: QVCLogDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QVCLogDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QVCLogDialog::updateClicked()
{
	form_->update_contents();
}
