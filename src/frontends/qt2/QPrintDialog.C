/**
 * \file QPrintDialog.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>
 
#include <gettext.h>
#include "support/filetools.h"
#include "support/lstrings.h"

#include "QPrint.h"
#include "QPrintDialog.h"

#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>

QPrintDialog::QPrintDialog(QPrint * f)
	: QPrintDialogBase(0, 0, false, 0),
	form_(f)
{
	connect(printPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
}

void QPrintDialog::change_adaptor()
{
	form_->changed();
}


void QPrintDialog::browseClicked()
{
	QString file = QFileDialog::getOpenFileName(QString::null,
		_("PostScript files (*.ps)"), this, 0, _("Select a file to print to"));
	if (!file.isNull()) {
		fileED->setText(file);
		form_->changed();
	}
}


void QPrintDialog::fileChanged()
{
	if (!fileED->text().isEmpty())
		fileRB->setChecked(true);
	form_->changed();
}


void QPrintDialog::copiesChanged(int i)
{
	collateCB->setEnabled(i != 1);
	form_->changed();
}


void QPrintDialog::printerChanged()
{
	printerRB->setChecked(true);
	form_->changed();
}


void QPrintDialog::pagerangeChanged()
{
	int from = strToUnsignedInt(fromED->text().latin1());
	int to = strToUnsignedInt(toED->text().latin1());

	if (!toED->text().isEmpty() && from > to)
		fromED->setText(toED->text());

	form_->changed();
}
