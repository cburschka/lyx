/**
 * \file QChangesDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QChangesDialog.h"
#include "QChanges.h"

#include <qpushbutton.h>


namespace lyx {
namespace frontend {

QChangesDialog::QChangesDialog(QChanges * form)
	: QChangesDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QChangesDialog::nextPressed()
{
	form_->next();
}


void QChangesDialog::acceptPressed()
{
	form_->accept();
}


void QChangesDialog::rejectPressed()
{
	form_->reject();
}


void QChangesDialog::closeEvent(QCloseEvent *e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx
