/**
 * \file GuiPrint.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiPrint.h"
#include "GuiPrintDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "PrinterParams.h"

#include "controllers/ControlPrint.h"

#include "support/os.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QPushButton>

using lyx::support::os::internal_path;

namespace lyx {
namespace frontend {

typedef QController<ControlPrint, GuiView<GuiPrintDialog> > print_base_class;


GuiPrint::GuiPrint(Dialog & parent)
	: print_base_class(parent, _("Print Document"))
{
}


void GuiPrint::build_dialog()
{
	dialog_.reset(new GuiPrintDialog(this));

	bcview().setOK(dialog_->printPB);
	bcview().setCancel(dialog_->closePB);
}


void GuiPrint::update_contents()
{
	PrinterParams & pp = controller().params();

	// only reset params if a different buffer
	if (!pp.file_name.empty() && pp.file_name == fromqstr(dialog_->fileED->text()))
		return;

	dialog_->printerED->setText(toqstr(pp.printer_name));
	dialog_->fileED->setText(toqstr(pp.file_name));

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


void GuiPrint::apply()
{
	PrinterParams::Target t = PrinterParams::PRINTER;
	if (dialog_->fileRB->isChecked())
		t = PrinterParams::FILE;

	PrinterParams const pp(t,
		fromqstr(dialog_->printerED->text()),
		internal_path(fromqstr(dialog_->fileED->text())),
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

} // namespace frontend
} // namespace lyx
