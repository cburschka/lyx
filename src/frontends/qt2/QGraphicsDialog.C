/**
 * \file QGraphicsDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <vector>

#include <qwidget.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include "LString.h" 
 
#include "QGraphicsDialog.h"
#include "ControlGraphics.h" 
#include "Dialogs.h"
#include "QGraphics.h"
#include "debug.h" 

QGraphicsDialog::QGraphicsDialog(QGraphics * form)
	: QGraphicsDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}

 
void QGraphicsDialog::change_adaptor()
{
	form_->changed();
}

 
void QGraphicsDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QGraphicsDialog::browseClicked()
{
	form_->browse();
}
