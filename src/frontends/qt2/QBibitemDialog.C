/**
 * \file QBibitemDialog.C
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

#include "ControlBibitem.h"

#include <qpushbutton.h>
#include <qlineedit.h>

#include "QBibitemDialog.h"
#include "QBibitem.h"


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
