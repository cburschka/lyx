/**
 * \file printdlgimpl.C
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#include "qfiledialog.h"
#include "qcheckbox.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include "qradiobutton.h"
#include "qspinbox.h"

#include "printdlgimpl.h"
#include "FormPrint.h"

#include <config.h>

#include "support/filetools.h"
#include "support/lstrings.h"
#include "lyxrc.h" 
#include "PrinterParams.h"

#include <gettext.h>
#include <cstring>


PrintDlgImpl::PrintDlgImpl( FormPrint *f, QWidget* parent,  const char* name, bool modal, WFlags fl )
	: PrintDlg( parent, name, modal, fl ), form_(f)
{
	setCaption(name);
}

 
PrintDlgImpl::~PrintDlgImpl()
{
	 // no need to delete child widgets, Qt does it all for us
}


const char * PrintDlgImpl::getFrom() {
	return fromPage->text();
}


const char * PrintDlgImpl::getTo() {
	return toPage->text();
}

	
PrinterParams::Target PrintDlgImpl::getTarget() {
	if (toPrinter->isChecked())
		return PrinterParams::PRINTER;
	else
		return PrinterParams::FILE;
}

	
const char * PrintDlgImpl::getPrinter() {
	return printerName->text();
}

const char * PrintDlgImpl::getFile() {
	return fileName->text();
}

PrinterParams::WhichPages PrintDlgImpl::getWhichPages() {
	if (oddPages->isChecked())
		return PrinterParams::ODD;
	else if (evenPages->isChecked())
		return PrinterParams::EVEN;
	else
		return PrinterParams::ALL;
}

bool PrintDlgImpl::getReverse() {
	return reverse->isChecked();
}

bool PrintDlgImpl::getSort() {
	return collate->isChecked();
}

const char * PrintDlgImpl::getCount() {
	return copies->text();
}

void PrintDlgImpl::setTarget(PrinterParams::Target t) {
	toPrinter->setChecked(t==PrinterParams::PRINTER);
	toFile->setChecked(t!=PrinterParams::PRINTER);
}

void PrintDlgImpl::setPrinter(const char * name) {
	printerName->setText(name);
}

void PrintDlgImpl::setFile(const char * name) {
	fileName->setText(name);
}	 

void PrintDlgImpl::setWhichPages(PrinterParams::WhichPages wp) {
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


void PrintDlgImpl::setReverse(bool on) {
	reverse->setChecked(on);
}


void PrintDlgImpl::setSort(bool on) {
	collate->setChecked(on);
}


void PrintDlgImpl::setCount(int num) {
	copies->setValue(num);
	collate->setEnabled(num > 1);
}


void PrintDlgImpl::setFrom(const char * text) {
	fromPage->setText(text);
}


void PrintDlgImpl::setTo(const char * text) {
	toPage->setText(text);
}

 
void PrintDlgImpl::browse_file()
{
	QString d( OnlyPath(tostr(fileName->text())).c_str() );
	QString s( QFileDialog::getOpenFileName( d, "PostScript Files (*.ps)", this ) );
	if (!s.isNull()) 
		fileName->setText(s);
}

 
void PrintDlgImpl::print()
{
	form_->print();
	form_->close();
	hide();
}

 
void PrintDlgImpl::cancel_adaptor()
{
	form_->close();
	hide();
}

 
void PrintDlgImpl::set_collate(int copies)
{
	collate->setEnabled(copies > 1);
}
