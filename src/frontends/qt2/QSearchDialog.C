/**
 * \file QSearchDialog.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include "ControlSearch.h"
#include "QSearchDialog.h"
#include "debug.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>

QSearchDialog::QSearchDialog(QSearch * form)
	: QSearchDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}


void QSearchDialog::show()
{
	QSearchDialogBase::show();
	findCO->setFocus();
	findCO->lineEdit()->setSelection(0, findCO->lineEdit()->text().length());
}

 
void QSearchDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QSearchDialog::findChanged()
{
	if (findCO->currentText().isEmpty()) {
		findPB->setEnabled(false);
		replacePB->setEnabled(false);
		replaceallPB->setEnabled(false);
	} else {
		findPB->setEnabled(true);
		replacePB->setEnabled(!form_->readOnly());
		replaceallPB->setEnabled(!form_->readOnly());
	}
}


void QSearchDialog::findClicked()
{
	string const find(findCO->currentText().latin1());
	form_->find(find,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		backwardsCB->isChecked());
	findCO->insertItem(findCO->currentText());
}


void QSearchDialog::replaceClicked()
{
	string const find(findCO->currentText().latin1());
	string const replace(replaceCO->currentText().latin1());
	form_->replace(find, replace,
		caseCB->isChecked(),
		wordsCB->isChecked(),
		false);
	replaceCO->insertItem(replaceCO->currentText());
}


void QSearchDialog::replaceallClicked()
{
	form_->replace(findCO->currentText().latin1(),
		replaceCO->currentText().latin1(),
		caseCB->isChecked(),
		wordsCB->isChecked(),
		true);
}
