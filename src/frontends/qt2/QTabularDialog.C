/**
 * \file QTabularDialog.C
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

#include "ControlTabular.h"
 
#include "QTabular.h"
#include "QTabularDialog.h"
#include "tabular.h"

#include <qpushbutton.h>


QTabularDialog::QTabularDialog(QTabular * form)
	: QTabularDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QTabularDialog::change_adaptor()
{
	form_->changed();
}


void QTabularDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
 

void QTabularDialog::columnAppend_clicked()
{
	form_->controller().set(LyXTabular::APPEND_COLUMN);
}

 
void QTabularDialog::rowAppend_clicked()
{
	form_->controller().set(LyXTabular::APPEND_ROW);
}

 
void QTabularDialog::columnDelete_clicked()
{
	form_->controller().set(LyXTabular::DELETE_COLUMN);
}

 
void QTabularDialog::rowDelete_clicked()
{
	form_->controller().set(LyXTabular::DELETE_COLUMN);
}
