/**
 * \file QBranchDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QBranchDialog.h"
#include "QBranch.h"

#include <qpushbutton.h>

namespace lyx {
namespace frontend {

QBranchDialog::QBranchDialog(QBranch * form)
	: QBranchDialogBase(qApp->focusWidget() ? qApp->focusWidget() : qApp->mainWidget(), 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QBranchDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QBranchDialog::change_adaptor()
{
	form_->changed();
}

} // namespace frontend
} // namespace lyx
