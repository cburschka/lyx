/**
 * \file GuiPrint.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiPrint.h"

#include "qt_helpers.h"
#include "PrinterParams.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "FuncRequest.h"
#include "LyXRC.h"

#include "support/convert.h"
#include "support/filetools.h"
#include "support/os.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QPushButton>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiPrint::GuiPrint(GuiView & lv)
	: GuiDialog(lv, "print", qt_("Print Document"))
{
	setupUi(this);

	connect(printPB, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));

	connect(copiesSB, SIGNAL(valueChanged(int)), this, SLOT(copiesChanged(int)));
	connect(printerED, SIGNAL(textChanged(QString)),
		this, SLOT(printerChanged()));
	connect(fileED, SIGNAL(textChanged(QString)),
		this, SLOT(fileChanged() ));
	connect(browsePB, SIGNAL(clicked()), this, SLOT(browseClicked()));
	connect(allRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(reverseCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(collateCB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(fromED, SIGNAL(textChanged(QString)),
		this, SLOT(pagerangeChanged()));
	connect(fromED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(toED, SIGNAL(textChanged(QString)),
		this, SLOT(pagerangeChanged()));
	connect(toED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
	connect(fileRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(printerRB, SIGNAL(clicked()), this, SLOT(change_adaptor()));
	connect(rangeRB, SIGNAL(toggled(bool)), fromED, SLOT(setEnabled(bool)));
	connect(rangeRB, SIGNAL(toggled(bool)), toED, SLOT(setEnabled(bool)));

	bc().setPolicy(ButtonPolicy::OkApplyCancelPolicy);
	bc().setOK(printPB);
	bc().setCancel(closePB);
}


void GuiPrint::change_adaptor()
{
	changed();
}


void GuiPrint::browseClicked()
{
	QString file =
		browseRelFile(QString(), bufferFilepath(), qt_("Print to file"),
			     QStringList(qt_("PostScript files (*.ps)")), true);
	if (!file.isEmpty()) {
		fileED->setText(file);
		changed();
	}
}


void GuiPrint::fileChanged()
{
	if (!fileED->text().isEmpty())
		fileRB->setChecked(true);
	changed();
}


void GuiPrint::copiesChanged(int i)
{
	collateCB->setEnabled(i != 1);
	changed();
}


void GuiPrint::printerChanged()
{
	printerRB->setChecked(true);
	changed();
}


void GuiPrint::pagerangeChanged()
{
	changed();
}


void GuiPrint::updateContents()
{
	// only reset params if a different buffer
	if (!params_.file_name.empty()
			&& params_.file_name == fromqstr(fileED->text()))
		return;

	printerED->setText(toqstr(params_.printer_name));
	fileED->setText(toqstr(params_.file_name));

	printerRB->setChecked(true);
	if (params_.target == PrinterParams::FILE)
		fileRB->setChecked(true);

	reverseCB->setChecked(params_.reverse_order);

	copiesSB->setValue(params_.count_copies);

	oddCB->setChecked(params_.odd_pages);
	evenCB->setChecked(params_.even_pages);

	collateCB->setChecked(params_.sorted_copies);

	if (params_.all_pages) {
		allRB->setChecked(true);
	} else {
		rangeRB->setChecked(true);
		fromED->setText(QString::number(params_.from_page));
		toED->setText(QString::number(params_.to_page));
	}
}


void GuiPrint::applyView()
{
	params_.target        = fileRB->isChecked()
		?  PrinterParams::FILE : PrinterParams::PRINTER;
	params_.printer_name  = fromqstr(printerED->text());
	params_.file_name     = os::internal_path(fromqstr(fileED->text()));
	params_.all_pages     = allRB->isChecked();
	params_.from_page     = fromED->text().toUInt();
	params_.to_page       = toED->text().toUInt();
	params_.odd_pages     = oddCB->isChecked();
	params_.even_pages    = evenCB->isChecked();
	params_.count_copies  = copiesSB->text().toUInt();
	params_.sorted_copies = collateCB->isChecked();
	params_.reverse_order = reverseCB->isChecked();
}


bool GuiPrint::initialiseParams(string const &)
{
	/// get global printer parameters
	params_ = PrinterParams();
	params_.file_name = support::changeExtension(buffer().absFileName(),
					lyxrc.print_file_extension);

	setButtonsValid(true); // so that the user can press Ok
	return true;
}


void GuiPrint::clearParams()
{
	params_ = PrinterParams();
}


/// print the current buffer
void GuiPrint::dispatchParams()
{
	string command = lyxrc.print_command + ' ';

	if (params_.target == PrinterParams::PRINTER
	    && lyxrc.print_adapt_output  // dvips wants a printer name
	    && !params_.printer_name.empty()) {// printer name given
		command += lyxrc.print_to_printer + params_.printer_name + ' ';
	}

	if (!params_.all_pages && params_.from_page) {
		command += lyxrc.print_pagerange_flag + ' ';
		command += convert<string>(params_.from_page);
		if (params_.to_page) {
			// we have a range "from-to"
			command += '-' + convert<string>(params_.to_page);
		}
		command += ' ';
	}

	// If both are, or both are not selected, then skip the odd/even printing
	if (params_.odd_pages != params_.even_pages) {
		if (params_.odd_pages)
			command += lyxrc.print_oddpage_flag + ' ';
		else if (params_.even_pages)
			command += lyxrc.print_evenpage_flag + ' ';
	}

	if (params_.count_copies > 1) {
		if (params_.sorted_copies)
			command += lyxrc.print_collcopies_flag;
		else
			command += lyxrc.print_copies_flag;
		command += ' ' + convert<string>(params_.count_copies) + ' ';
	}

	if (params_.reverse_order)
		command += lyxrc.print_reverse_flag + ' ';

	if (!lyxrc.print_extra_options.empty())
		command += lyxrc.print_extra_options + ' ';

	command += buffer().params().dvips_options();

	string const target = (params_.target == PrinterParams::PRINTER) ?
		"printer" : "file";

	string const target_name = (params_.target == PrinterParams::PRINTER) ?
		(params_.printer_name.empty() ? "default" : params_.printer_name) :
		params_.file_name;

	string const data = target + " \"" + target_name + "\" \"" + command + '"';
	dispatch(FuncRequest(getLfun(), data));
}


Dialog * createGuiPrint(GuiView & lv) { return new GuiPrint(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiPrint.cpp"
