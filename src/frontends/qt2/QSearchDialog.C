/**
 * \file QSearchDialog.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include "QSearchDialog.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>

QSearchDialog::QSearchDialog(QSearch * form, QWidget * parent,  const char * name, bool modal, WFlags fl)
	: QSearchDialogBase(parent, name, modal, fl),
	form_(form)
{
	setCaption(name);
}

 
QSearchDialog::~QSearchDialog()
{
}


void QSearchDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}


void QSearchDialog::setReadOnly(bool readonly)
{
	replace->setEnabled(!readonly);
	replaceLabel->setEnabled(!readonly);
	replacePB->setEnabled(!readonly);
	replaceAllPB->setEnabled(!readonly);
}


void QSearchDialog::Find()
{
	form_->find(tostr(find->currentText()).c_str(),
			 caseSensitive->isChecked(),
			 matchWord->isChecked(),
			 !searchBack->isChecked());
}


void QSearchDialog::Replace(bool replaceall)
{
	form_->replace(tostr(find->currentText()).c_str(),
		  tostr(replace->currentText()).c_str(),
		  caseSensitive->isChecked(),
		  matchWord->isChecked(),
		  !searchBack->isChecked(),
		  replaceall);
}
