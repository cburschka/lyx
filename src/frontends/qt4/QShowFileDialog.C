/**
 * \file QShowFileDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QShowFileDialog.h"
#include "QShowFile.h"

#include <QPushButton>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QShowFileDialog::QShowFileDialog(QShowFile * form)
	: form_(form)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
}


void QShowFileDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "QShowFileDialog_moc.cpp"
