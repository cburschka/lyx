/**
 * \file printdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef PRINTDLG_H
#define PRINTDLG_H


#include "dlg/printdlgdata.h"
#include "support/lstrings.h"
#include "lyxrc.h" 
#include "PrinterParams.h"
#include "FormPrint.h"

class PrintDialog : public PrintDialogData
{
	Q_OBJECT
public:

	PrintDialog(FormPrint * f, QWidget * parent = NULL, char const * name = NULL);
	virtual ~PrintDialog();
	
	char const * getFrom() {
		return from->text();
	}

	char const * getTo() {
		return to->text();
	}

	PrinterParams::Target getTarget() {
		if (toprinter->isChecked())
			return PrinterParams::PRINTER;
		else
			return PrinterParams::FILE;
	}

	char const * getPrinter() {
		return printername->text();
	}

	char const * getFile() {
		return filename->text();
	}
	
	PrinterParams::WhichPages getWhichPages() {
		if (oddpages->isChecked())
			return PrinterParams::ODD;
		else if (evenpages->isChecked())
			return PrinterParams::EVEN;
		else
			return PrinterParams::ALL;
	}

	bool getReverse() {
		return reverse->isChecked();
	}

	bool getSort() {
		return sort->isChecked();
	}

	char const * getCount() {
		return count->text();
	}

	void setTarget(PrinterParams::Target t) {
		toprinter->setChecked(t == PrinterParams::PRINTER);
		tofile->setChecked(t != PrinterParams::PRINTER);
	}
	 
       	void setPrinter(char const * name) {
		printername->setText(name);
	} 
 
	void setFile(char const * name) {
		filename->setText(name);
	}	 
 
	void setWhichPages(PrinterParams::WhichPages wp) {
		switch (wp) {
			case PrinterParams::ALL:
				allpages->setChecked(true);
				break;
			case PrinterParams::EVEN:
				evenpages->setChecked(true);
				break;
			case PrinterParams::ODD:
				oddpages->setChecked(true);
				break;
		}
	}
 
	void setReverse(bool on) {
		reverse->setChecked(on);
	}
 
	void setSort(bool on) {
		sort->setChecked(on);
	}
	 
	void setCount(int num) {
		count->setText(tostr(num).c_str());
		sort->setEnabled(num > 1);
	}

	void setFrom(char const * text) {
		from->setText(text);
	}

	void setTo(char const * text) {
		to->setText(text);
	}

protected slots:
 
	void clickedCancel() {
		form_->close();
		hide();
	}
 
	void clickedPrint() {
		form_->print(); 
		form_->close();
		hide();
	}

	/// open up the browser to select ps file 
	void clickedBrowse();
	
	/// validate and change collate status
	void changedCount(char const * text);
 
private:
    	FormPrint * form_;
};
#endif // PRINTDLG_H
