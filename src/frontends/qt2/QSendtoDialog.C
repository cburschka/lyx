/**
 * \file QSendtoDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QSendtoDialog.h"
#include "QSendto.h"

#include <qpushbutton.h>


namespace lyx {
namespace frontend {

QSendtoDialog::QSendtoDialog(QSendto * form)
	: QSendtoDialogBase(qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget(), 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QSendtoDialog::changed_adaptor()
{
	form_->changed();
}


void QSendtoDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
