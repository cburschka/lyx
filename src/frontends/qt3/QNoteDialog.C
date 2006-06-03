/**
 * \file QNoteDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QNoteDialog.h"
#include "QNote.h"

#include <qpushbutton.h>
#include <qradiobutton.h>

namespace lyx {
namespace frontend {

QNoteDialog::QNoteDialog(QNote * form)
	: QNoteDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(okPB, SIGNAL(clicked()),
		form, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(framedRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
	connect(shadedRB, SIGNAL(toggled(bool)), this, SLOT(change_adaptor()));
}

void QNoteDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QNoteDialog::change_adaptor()
{
	form_->changed();
}

} // namespace frontend
} // namespace lyx
