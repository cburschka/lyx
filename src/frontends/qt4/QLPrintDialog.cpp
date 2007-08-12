/**
 * \file QLPrintDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "controllers/ControlPrint.h"
#include "QLPrintDialog.h"
#include "QPrint.h"
#include "qt_helpers.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

namespace lyx {
namespace frontend {

QLPrintDialog::QLPrintDialog(QPrint * f)
	: form_(f)
{
	setupUi(this);

	connect(printPB, SIGNAL(clicked()), form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));

	connect(copiesSB, SIGNAL(valueChanged(int)), this, SLOT(copiesChanged(int)));
	connect(printerED, SIGNAL(textChanged(const QString&)),
		this, SLOT(printerChanged()));
	connect(fileED, SIGNAL(textChanged(const QString&)),
		this, SLOT(fileChanged() ));
	connect(browsePB, SIGNAL(clicked()), this, SLOT(browseClicked()));
	connect(allRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(reverseCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(collateCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(fromED, SIGNAL(textChanged(const QString&)),
		this, SLOT(pagerangeChanged()));
	connect(fromED, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(toED, SIGNAL(textChanged(const QString&)),
		this, SLOT(pagerangeChanged()));
	connect(toED, SIGNAL(textChanged(const QString&)),
		this, SLOT(change_adaptor()));
	connect(fileRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(printerRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(rangeRB, SIGNAL(toggled(bool)), fromED, SLOT(setEnabled(bool)));
	connect(rangeRB, SIGNAL(toggled(bool)), toED, SLOT(setEnabled(bool)));
}


void QLPrintDialog::change_adaptor()
{
	form_->changed();
}


void QLPrintDialog::browseClicked()
{
	QString file = toqstr(form_->controller().browse(docstring()));
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

#include "QLPrintDialog_moc.cpp"
