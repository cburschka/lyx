/**
 * \file QLogDialog.C
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

#include "LyXView.h"
#include "ControlLog.h"

#include <qpushbutton.h>
#include <qtextview.h>

#include "QLogDialog.h"
#include "QLog.h"

#include <vector>


QLogDialog::QLogDialog(QLog * form)
	: QLogDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QLogDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QLogDialog::updateClicked()
{
	form_->update_contents();
}
