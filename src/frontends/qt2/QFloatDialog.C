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
#include <qcheckbox.h>

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

void QFloatDialog::tbhpClicked()
{
	heredefinitelyCB->setChecked(false);
	bool allow(topCB->isChecked());
	allow |= bottomCB->isChecked();
	allow |= pageCB->isChecked();
	allow |= herepossiblyCB->isChecked();
	ignoreCB->setEnabled(allow);
}
 

void QFloatDialog::heredefinitelyClicked()
{
	if (heredefinitelyCB->isChecked());
		ignoreCB->setEnabled(false);

	topCB->setChecked(false);
	bottomCB->setChecked(false);
	pageCB->setChecked(false);
	herepossiblyCB->setChecked(false);
	ignoreCB->setChecked(false);
}


void QFloatDialog::spanClicked()
{
	bool const span(spanCB->isChecked());

	if (!defaultsCB->isChecked()) {
	 	herepossiblyCB->setEnabled(!span);
	 	heredefinitelyCB->setEnabled(!span);
	}

	if (!span)
		return;
 
	herepossiblyCB->setChecked(false);
	heredefinitelyCB->setChecked(false);
}
