/**
 * \file QRefDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#include "QRefDialog.h"
#include "Dialogs.h"
#include "QRef.h"

QRefDialog::QRefDialog(QRef * form, QWidget * parent, const char * name, bool modal, WFlags fl)
	: QRefDialogBase(parent, name, modal, fl),
	form_(form)
{
}


QRefDialog::~QRefDialog()
{
}

 
void QRefDialog::apply_adaptor()
{
	form_->apply();
	form_->close();
	hide();
}


void QRefDialog::goto_adaptor()
{
	form_->goto_ref();
}


void QRefDialog::highlight_adaptor(const QString & sel)
{
	form_->highlight(sel); 
}


void QRefDialog::close_adaptor()
{
	form_->close();
	hide();
}


void QRefDialog::select_adaptor(const QString & sel)
{
	form_->select(sel);
}


void QRefDialog::sort_adaptor(bool sort)
{
	form_->set_sort(sort);
}


void QRefDialog::update_adaptor()
{
	form_->do_ref_update();
}



void QRefDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}
