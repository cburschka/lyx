/**
 * \file QLPrintDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QLPrintDialog.h"
#include "QPrint.h"
#include "qt_helpers.h"

#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

namespace lyx {
namespace frontend {

QLPrintDialog::QLPrintDialog(QPrint * f)
	: QPrintDialogBase(0, 0, false, 0),
	form_(f)
{
	connect(printPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}


void QLPrintDialog::change_adaptor()
{
	form_->changed();
}


void QLPrintDialog::browseClicked()
{
	QString file =
		QFileDialog::getOpenFileName(QString::null,
					     qt_("PostScript files (*.ps)"),
					     this, 0,
					     qt_("Select a file to print to"));
	if (!file.isNull()) {
		fileED->setText(file);
		form_->changed();
	}
}


void QLPrintDialog::fileChanged()
{
	if (!fileED->text().isEmpty())
		fileRB->setChecked(true);
	form_->changed();
}


void QLPrintDialog::copiesChanged(int i)
{
	collateCB->setEnabled(i != 1);
	form_->changed();
}


void QLPrintDialog::printerChanged()
{
	printerRB->setChecked(true);
	form_->changed();
}


void QLPrintDialog::pagerangeChanged()
{
	form_->changed();
}

} // namespace frontend
} // namespace lyx
