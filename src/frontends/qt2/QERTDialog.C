/**
 * \file QERTDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QERTDialog.h"
#include "QERT.h"

#include <qpushbutton.h>

namespace lyx {
namespace frontend {

QERTDialog::QERTDialog(QERT * form)
	: QERTDialogBase(qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget(), 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QERTDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QERTDialog::change_adaptor()
{
	form_->changed();
}

} // namespace frontend
} // namespace lyx
