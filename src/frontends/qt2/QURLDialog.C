/**
 * \file QURLDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>
 
#include "QURLDialog.h"
#include "Dialogs.h"
#include "QURL.h"

QURLDialog::QURLDialog(QURL * form, QWidget * parent,  const char * name, bool modal, WFlags fl)
	: QURLDialogBase(parent, name, modal, fl),
	form_(form)
{
}

 
QURLDialog::~QURLDialog()
{
}

 
void QURLDialog::apply_adaptor()
{
	form_->apply();
	form_->close();
	hide();
}


void QURLDialog::close_adaptor()
{
	form_->close();
	hide();
}

 
void QURLDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}
