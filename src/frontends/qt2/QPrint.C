/**
 * \file QPrint.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qpushbutton.h>

#include "QPrintDialog.h"
#include "QPrint.h"
#include "Qt2BC.h"
#include "gettext.h"

#include "QtLyXView.h"
#include "ControlPrint.h"

#include "support/lstrings.h"

typedef Qt2CB<ControlPrint, Qt2DB<QPrintDialog> > base_class;

QPrint::QPrint(ControlPrint & c)
	: base_class(c, _("Print"))
{
}


void QPrint::build_dialog()
{
	dialog_.reset(new QPrintDialog(this));

	bc().setOK(dialog_->printPB);
	bc().setCancel(dialog_->closePB);
}


void QPrint::update_contents()
{
	PrinterParams & pp = controller().params();

	dialog_->printerED->setText(pp.printer_name.c_str());
	dialog_->fileED->setText(pp.file_name.c_str());

	dialog_->printerRB->setChecked(true);
	if (pp.target == PrinterParams::FILE)
		dialog_->fileRB->setChecked(true);

	dialog_->reverseCB->setChecked(pp.reverse_order);

	QRadioButton * button;
	switch (pp.which_pages) {
		case PrinterParams::ALL: button = dialog_->allRB; break;
		case PrinterParams::ODD: button = dialog_->oddRB; break;
		case PrinterParams::EVEN: button = dialog_->evenRB; break;
	}
	button->setChecked(true);

	// hmmm... maybe a bit weird but maybe not
	// we might just be remembering the last
	// time this was printed.
	if (!pp.from_page.empty()) {
		dialog_->fromED->setText(pp.from_page.c_str());

		dialog_->toED->setText("");
		if (pp.to_page)
			dialog_->toED->setText(tostr(pp.to_page).c_str());
	} else {
		dialog_->fromED->setText("");
		dialog_->toED->setText("");
	}

	dialog_->copiesSB->setValue(pp.count_copies);
}


void QPrint::apply()
{
	PrinterParams::WhichPages wp;

	if (dialog_->allRB->isChecked())
		wp = PrinterParams::ALL;
	else if (dialog_->oddRB->isChecked())
		wp = PrinterParams::ODD;
	else
		wp = PrinterParams::EVEN;

	string from;
	int to(0);
	if (!dialog_->fromED->text().isEmpty()) {
		// we have at least one page requested
		from = dialog_->fromED->text().latin1();
		if (!dialog_->toED->text().isEmpty())
			to = strToInt(dialog_->toED->text().latin1());
	}

	PrinterParams::Target t = PrinterParams::PRINTER;
	if (dialog_->fileRB->isChecked())
		t = PrinterParams::FILE;

	PrinterParams const pp(t,
		dialog_->printerED->text().latin1(),
		dialog_->fileED->text().latin1(),
		wp, from, to,
		dialog_->reverseCB->isChecked(),
		!dialog_->collateCB->isChecked(),
		strToInt(dialog_->copiesSB->text().latin1()));

	controller().params() = pp;
}
