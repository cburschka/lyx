/**
 * \file QBibitemDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <qpushbutton.h>
#include <qlineedit.h>
 
#include "QBibitemDialog.h"
#include "QBibitem.h"
#include "Dialogs.h"
#include "ControlBibitem.h" 

QBibitemDialog::QBibitemDialog(QBibitem * form)
	: QBibitemDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}

 
void QBibitemDialog::change_adaptor()
{
	form_->changed();
}


void QBibitemDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}
