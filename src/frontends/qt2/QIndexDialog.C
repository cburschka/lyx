/**
 * \file QIndexDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include "QIndexDialog.h"
#include "Dialogs.h"
#include "QIndex.h"

QIndexDialog::QIndexDialog(QIndex * form, QWidget * parent, const char * name, bool modal, WFlags fl)
	: QIndexDialogBase(parent, name, modal, fl),
	form_(form)
{
}

 
QIndexDialog::~QIndexDialog()
{
}


void QIndexDialog::apply_adaptor()
{
	form_->apply();
	form_->close();
	hide();
}


void QIndexDialog::close_adaptor()
{
	form_->close();
	hide();
}


void QIndexDialog::closeEvent(QCloseEvent *e)
{
	form_->close();
	e->accept();
}
