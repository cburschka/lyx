/**
 * \file QTocDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include "QTocDialog.h"
#include "Dialogs.h"
#include "QToc.h"

#include <qlistview.h>
#include <qpushbutton.h>

QTocDialog::QTocDialog(QToc * form)
	: QTocDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


QTocDialog::~QTocDialog()
{
}


void QTocDialog::activate_adaptor(int)
{
	form_->updateToc(form_->depth_);
}


void QTocDialog::depth_adaptor(int depth)
{
	form_->set_depth(depth);
}


void QTocDialog::select_adaptor(QListViewItem * item)
{
	form_->select(item->text(0).latin1());
}


void QTocDialog::update_adaptor()
{
	form_->update();
}


void QTocDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}
