/**
 * \file searchdlgimpl.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven
 */

#include <config.h>

#include "searchdlgimpl.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>


SearchDlgImpl::SearchDlgImpl(FormSearch* form, QWidget* parent,  const char* name, bool modal, WFlags fl )
    : SearchDlg( parent, name, modal, fl ), form_(form)
{
   setCaption(name);
}

SearchDlgImpl::~SearchDlgImpl()
{
}

void SearchDlgImpl::closeEvent(QCloseEvent * e)
{
   form_->close();
   e->accept();
}

void SearchDlgImpl::setReadOnly(bool readonly)
{
   replace->setEnabled(!readonly);
   replaceLabel->setEnabled(!readonly);
   replacePB->setEnabled(!readonly);
   replaceAllPB->setEnabled(!readonly);
}

void SearchDlgImpl::Find()
{
   form_->find(tostr(find->currentText()).c_str(),
	       caseSensitive->isChecked(),
	       matchWord->isChecked(),
	       !searchBack->isChecked());
}

void SearchDlgImpl::Replace(bool replaceall = false)
{
   form_->replace(tostr(find->currentText()).c_str(),
		  tostr(replace->currentText()).c_str(),
		  caseSensitive->isChecked(),
		  matchWord->isChecked(),
		  !searchBack->isChecked(),
		  replaceall);
}


