/**
 * \file QIndexDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include "QIndexDialog.h"
#include "Dialogs.h"
#include "QIndex.h"

#include <qwidget.h>
#include <qpushbutton.h>

QIndexDialog::QIndexDialog(QIndex * form)
	: QIndexDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}

 
void QIndexDialog::change_adaptor()
{
	form_->changed();
}

 
void QIndexDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
