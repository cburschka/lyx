/**
 * \file QFloatDialog.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>

#include "LString.h"

#include "ControlFloat.h"

#include <qpushbutton.h>

#include "QFloatDialog.h"
#include "QFloat.h"
 
QFloatDialog::QFloatDialog(QFloat * form)
	: QFloatDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(restorePB, SIGNAL(clicked()),
		form, SLOT(slotRestore()));
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QFloatDialog::change_adaptor()
{
	form_->changed();
}


void QFloatDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
