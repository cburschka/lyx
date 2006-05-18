/**
 * \file QLogDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QLogDialog.h"
#include "QLog.h"

#include <QPushButton>
#include <QCloseEvent>


namespace lyx {
namespace frontend {

QLogDialog::QLogDialog(QLog * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect( updatePB, SIGNAL( clicked() ), 
		this, SLOT( updateClicked() ) );
}


void QLogDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QLogDialog::updateClicked()
{
	form_->update_contents();
}

} // namespace frontend
} // namespace lyx

#include "QLogDialog_moc.cpp"
