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

#include "ControlPrint.h"
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

GuiPrintDialog::GuiPrintDialog(LyXView & lv)
	: GuiDialog(lv, "print")
{
	setupUi(this);
	setController(new ControlPrint(*this));
	setViewTitle(_("Print Document"));

	connect(printPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

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

	bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	bc().setOK(printPB);
	bc().setCancel(closePB);
}


ControlPrint & GuiPrintDialog::controller() const
{
	return static_cast<ControlPrint &>(Dialog::controller());
}


void GuiPrintDialog::change_adaptor()
{
	changed();
}


void GuiPrintDialog::browseClicked()
{
	QString file = toqstr(controller().browse(docstring()));
	if (!file.isNull()) {
		fileED->setText(file);
		changed();
	}
}


void GuiPrintDialog::fileChanged()
{
	if (!fileED->text().isEmpty())
		fileRB->setChecked(true);
	changed();
}


void GuiPrintDialog::copiesChanged(int i)
{
	collateCB->setEnabled(i != 1);
	changed();
}


void GuiPrintDialog::printerChanged()
{
	printerRB->setChecked(true);
	changed();
}


void GuiPrintDialog::pagerangeChanged()
{
	changed();
}


void GuiPrintDialog::update_contents()
{
	PrinterParams & pp = controller().params();

	// only reset params if a different buffer
	if (!pp.file_name.empty() && pp.file_name == fromqstr(fileED->text()))
		return;

	printerED->setText(toqstr(pp.printer_name));
	fileED->setText(toqstr(pp.file_name));

	printerRB->setChecked(true);
	if (pp.target == PrinterParams::FILE)
		fileRB->setChecked(true);

	reverseCB->setChecked(pp.reverse_order);

	copiesSB->setValue(pp.count_copies);

	oddCB->setChecked(pp.odd_pages);
	evenCB->setChecked(pp.even_pages);

	collateCB->setChecked(pp.sorted_copies);

	if (pp.all_pages) {
		allRB->setChecked(true);
	} else {
		rangeRB->setChecked(true);
		fromED->setText(QString::number(pp.from_page));
		toED->setText(QString::number(pp.to_page));
	}
}


void GuiPrintDialog::applyView()
{
	PrinterParams::Target t = PrinterParams::PRINTER;
	if (fileRB->isChecked())
		t = PrinterParams::FILE;

	PrinterParams const pp(t,
		fromqstr(printerED->text()),
		support::os::internal_path(fromqstr(fileED->text())),
		allRB->isChecked(),
		fromED->text().toUInt(),
		toED->text().toUInt(),
		oddCB->isChecked(),
		evenCB->isChecked(),
		copiesSB->text().toUInt(),
		collateCB->isChecked(),
		reverseCB->isChecked());

	controller().params() = pp;
}

} // namespace frontend
} // namespace lyx

#include "GuiPrint_moc.cpp"
