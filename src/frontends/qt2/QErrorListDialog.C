/**
 * \file QErrorListDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "QErrorList.h"
#include "QErrorListDialog.h"
#include "qt_helpers.h"

#include <qlistbox.h>
#include <qtextbrowser.h>
#include <qpushbutton.h>


QErrorListDialog::QErrorListDialog(QErrorList * form)
	: QErrorListDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(errorsLB, SIGNAL(returnPressed(QListBoxItem *)),
		form, SLOT(slotClose()));
}


QErrorListDialog::~QErrorListDialog()
{}


void QErrorListDialog::select_adaptor(int item)
{
	form_->select(item);
}


void QErrorListDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
