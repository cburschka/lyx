/**
 * \file QPrintDialog.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include <qfiledialog.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include "QPrintDialog.h"
#include "QPrint.h"

#include <config.h>

#include "support/filetools.h"
#include "support/lstrings.h"
#include "lyxrc.h" 
#include "PrinterParams.h"

#include <gettext.h>
#include <cstring>


QPrintDialog::QPrintDialog(QPrint * f, QWidget * parent,  const char * name, bool modal, WFlags fl)
	: QPrintDialogBase(parent, name, modal, fl), 
	form_(f)
{
	setCaption(name);
}

 
QPrintDialog::~QPrintDialog()
{
}


const char * QPrintDialog::getFrom() {
	return fromPage->text();
}


const char * QPrintDialog::getTo() {
	return toPage->text();
}

	
PrinterParams::Target QPrintDialog::getTarget() {
	if (toPrinter->isChecked())
		return PrinterParams::PRINTER;
	else
		return PrinterParams::FILE;
}


const char * QPrintDialog::getPrinter() {
	return printerName->text();
}


const char * QPrintDialog::getFile() {
	return fileName->text();
}


PrinterParams::WhichPages QPrintDialog::getWhichPages() {
	if (oddPages->isChecked())
		return PrinterParams::ODD;
	else if (evenPages->isChecked())
		return PrinterParams::EVEN;
	else
		return PrinterParams::ALL;
}


bool QPrintDialog::getReverse() {
	return reverse->isChecked();
}


bool QPrintDialog::getSort() {
	return collate->isChecked();
}


const char * QPrintDialog::getCount() {
	return copies->text();
}


void QPrintDialog::setTarget(PrinterParams::Target t) {
	toPrinter->setChecked(t==PrinterParams::PRINTER);
	toFile->setChecked(t!=PrinterParams::PRINTER);
}


void QPrintDialog::setPrinter(const char * name) {
	printerName->setText(name);
}


void QPrintDialog::setFile(const char * name) {
	fileName->setText(name);
}	 


void QPrintDialog::setWhichPages(PrinterParams::WhichPages wp) {
	switch (wp) {
		case PrinterParams::ALL:
			allPages->setChecked(true);
			break;
		case PrinterParams::EVEN:
			evenPages->setChecked(true);
			break;
		case PrinterParams::ODD:
			oddPages->setChecked(true);
			break;
	}
}


void QPrintDialog::setReverse(bool on) {
	reverse->setChecked(on);
}


void QPrintDialog::setSort(bool on) {
	collate->setChecked(on);
}


void QPrintDialog::setCount(int num) {
	copies->setValue(num);
	collate->setEnabled(num > 1);
}


void QPrintDialog::setFrom(const char * text) {
	fromPage->setText(text);
}


void QPrintDialog::setTo(const char * text) {
	toPage->setText(text);
}

 
void QPrintDialog::browse_file()
{
	QString d( OnlyPath(tostr(fileName->text())).c_str());
	QString s( QFileDialog::getOpenFileName(d, "PostScript Files (*.ps)", this));
	if (!s.isNull()) 
		fileName->setText(s);
}

 
void QPrintDialog::print()
{
	form_->print();
	form_->close();
	hide();
}

 
void QPrintDialog::cancel_adaptor()
{
	form_->close();
	hide();
}

 
void QPrintDialog::set_collate(int copies)
{
	collate->setEnabled(copies > 1);
}
