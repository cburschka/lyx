/**
 * \file QVCLogDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
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
