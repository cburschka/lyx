/**
 * \file QLogDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <vector>

#include <qpushbutton.h>
#include <qtextview.h>
#include "QLogDialog.h"
#include "Dialogs.h"
#include "QLog.h"

#include "QtLyXView.h"
#include "ControlLog.h"

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
