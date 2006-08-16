/**
 * \file QTocDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QTocDialog.h"
#include "QToc.h"
#include "qt_helpers.h"

#include <qlistview.h>
#include <qpushbutton.h>

namespace lyx {
namespace frontend {

QTocDialog::QTocDialog(QToc * form)
	: QTocDialogBase(0, 0, false, 0),
	form_(form)
{
	// disable sorting
	tocLV->setSorting(-1);

	// hide the pointless QHeader
	QWidget * w = static_cast<QWidget*>(tocLV->child("list view header"));
	if (w)
		w->hide();

	connect(closePB, SIGNAL(clicked()), form, SLOT(slotClose()));
	connect(moveupPB, SIGNAL(clicked()), this, SLOT(moveup_adaptor()));
	connect(movednPB, SIGNAL(clicked()), this, SLOT(movedn_adaptor()));
	connect(moveinPB, SIGNAL(clicked()), this, SLOT(movein_adaptor()));
	connect(moveoutPB, SIGNAL(clicked()), this, SLOT(moveout_adaptor()));
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


void QTocDialog::enableButtons(bool enable)
{
	updatePB->setEnabled(enable);

	if (!form_->canOutline())
		enable = false;

	moveupPB->setEnabled(enable);
	movednPB->setEnabled(enable);
	moveinPB->setEnabled(enable);
	moveoutPB->setEnabled(enable);
}


void QTocDialog::update_adaptor()
{
	form_->update();
}


void QTocDialog::moveup_adaptor()
{
	form_->moveup();
}


void QTocDialog::movedn_adaptor()
{
	form_->movedn();
}


void QTocDialog::movein_adaptor()
{
	form_->movein();
}


void QTocDialog::moveout_adaptor()
{
	form_->moveout();
}


void QTocDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "QTocDialog_moc.cpp"
