/**
 * \file QPrint.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LyXView.h"
#include "PrinterParams.h"
#include "ControlPrint.h"
#include "support/lstrings.h"
#include "gettext.h"

#include "QPrint.h"
#include "QPrintDialog.h"
#include "Qt2BC.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qpushbutton.h>

// FIXME FIXME QPrintDialog is getting destructed twice !!!!
 
typedef Qt2CB<ControlPrint, Qt2DB<QPrintDialog> > base_class;

QPrint::QPrint()
	: base_class(_("Print"))
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

	// only reset params if a different buffer
	if (!pp.file_name.empty() && pp.file_name == dialog_->fileED->text().latin1())
		return;
 
	dialog_->printerED->setText(pp.printer_name.c_str());
	dialog_->fileED->setText(pp.file_name.c_str());

	dialog_->printerRB->setChecked(true);
	if (pp.target == PrinterParams::FILE)
		dialog_->fileRB->setChecked(true);

	dialog_->reverseCB->setChecked(pp.reverse_order);

	dialog_->copiesSB->setValue(pp.count_copies);
 
	dialog_->oddCB->setChecked(pp.odd_pages);
	dialog_->evenCB->setChecked(pp.even_pages);
 
	dialog_->collateCB->setChecked(pp.sorted_copies);
 
	if (pp.all_pages) {
		dialog_->allRB->setChecked(true);
		return;
	}

	dialog_->rangeRB->setChecked(true);
 
	QString s;
	s.setNum(pp.from_page);
	dialog_->fromED->setText(s);
	s.setNum(pp.to_page);
	dialog_->toED->setText(s);
}


void QPrint::apply()
{
	PrinterParams::Target t = PrinterParams::PRINTER;
	if (dialog_->fileRB->isChecked())
		t = PrinterParams::FILE;

	PrinterParams const pp(t,
		dialog_->printerED->text().latin1(),
		dialog_->fileED->text().latin1(),
		dialog_->allRB->isChecked(), 
		dialog_->fromED->text().toUInt(),
		dialog_->toED->text().toUInt(), 
		dialog_->oddCB->isChecked(),
		dialog_->evenCB->isChecked(),
		dialog_->copiesSB->text().toUInt(),
		dialog_->collateCB->isChecked(),
		dialog_->reverseCB->isChecked());

	controller().params() = pp;
}
