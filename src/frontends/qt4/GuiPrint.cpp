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

#include "qt_helpers.h"
#include "PrinterParams.h"

#include "support/os.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QPushButton>

namespace lyx {
namespace frontend {

GuiPrintDialog::GuiPrintDialog(GuiPrint * f)
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


void GuiPrintDialog::change_adaptor()
{
	form_->changed();
}


void GuiPrintDialog::browseClicked()
{
	QString file = toqstr(form_->controller().browse(docstring()));
	if (!file.isNull()) {
		fileED->setText(file);
		form_->changed();
	}
}


void GuiPrintDialog::fileChanged()
{
	if (!fileED->text().isEmpty())
		fileRB->setChecked(true);
	form_->changed();
}


void GuiPrintDialog::copiesChanged(int i)
{
	collateCB->setEnabled(i != 1);
	form_->changed();
}


void GuiPrintDialog::printerChanged()
{
	printerRB->setChecked(true);
	form_->changed();
}


void GuiPrintDialog::pagerangeChanged()
{
	form_->changed();
}


GuiPrint::GuiPrint(GuiDialog & parent)
	: GuiView<GuiPrintDialog>(parent, _("Print Document"))
{
}


void GuiPrint::build_dialog()
{
	dialog_.reset(new GuiPrintDialog(this));

	bc().setOK(dialog_->printPB);
	bc().setCancel(dialog_->closePB);
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


void GuiPrint::applyView()
{
	PrinterParams::Target t = PrinterParams::PRINTER;
	if (dialog_->fileRB->isChecked())
		t = PrinterParams::FILE;

	PrinterParams const pp(t,
		fromqstr(dialog_->printerED->text()),
		support::os::internal_path(fromqstr(dialog_->fileED->text())),
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

#include "GuiPrint_moc.cpp"
