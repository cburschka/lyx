/**
 * \file QTocDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "QToc.h"
#include "QTocDialog.h"
#include "qt_helpers.h"

#include <qlistview.h>
#include <qpushbutton.h>


QTocDialog::QTocDialog(QToc * form)
	: QTocDialogBase(0, 0, false, 0),
	form_(form)
{
	// disable sorting
	tocLV->setSorting(-1);

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
	form_->select(fromqstr(item->text(0)));
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
