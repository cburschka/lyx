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
	: QTocDialogBase(qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget(), 0, false, 0),
	form_(form)
{
	// disable sorting
	tocLV->setSorting(-1);

	// hide the pointless QHeader
	QWidget * w = static_cast<QWidget*>(tocLV->child("list view header"));
	if (w)
		w->hide();

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

} // namespace frontend
} // namespace lyx
