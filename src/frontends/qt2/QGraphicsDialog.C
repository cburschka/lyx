/**
 * \file QGraphicsDialog.C
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

#include "ControlGraphics.h"
#include "debug.h"
#include "LString.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "QGraphicsDialog.h"
#include "QGraphics.h"

QGraphicsDialog::QGraphicsDialog(QGraphics * form)
	: QGraphicsDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
}


void QGraphicsDialog::show()
{ 
	QGraphicsDialogBase::show();
	filename->setFocus();
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


void QGraphicsDialog::browse_clicked()
{
	form_->browse();
}

void QGraphicsDialog::get_clicked()
{
	form_->get();
}
