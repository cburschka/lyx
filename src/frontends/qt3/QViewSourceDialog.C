/**
 * \file QViewSourceDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QViewSourceDialog.h"
#include "QViewSource.h"

#include <qpushbutton.h>
#include <qcheckbox.h>


namespace lyx {
namespace frontend {

QViewSourceDialog::QViewSourceDialog(QViewSource * form)
	: QViewSourceDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(viewFullSourceCB, SIGNAL(toggled(bool)),
		this, SLOT(slotUpdate()));
	connect(autoUpdateCB, SIGNAL(toggled(bool)),
		updatePB, SLOT(setDisabled(bool)));
	connect(updatePB, SIGNAL(clicked()),
		this, SLOT(slotUpdate()));
}


void QViewSourceDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QViewSourceDialog::slotUpdate()
{
	form_->update_source();
}

} // namespace frontend
} // namespace lyx

#include "QViewSourceDialog_moc.cpp"
