/**
 * \file printdlg.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef PRINTDLG_H
#define PRINTDLG_H


#include "FormPrint.h"
 
#include "dlg/printdlgdata.h"
 
#include "support/lstrings.h"
#include "lyxrc.h" 
#include "PrinterParams.h"
 
// FIXME: closeEvent 
 
class PrintDialog : public PrintDialogData
{
	Q_OBJECT
public:

	PrintDialog(FormPrint * f, QWidget * parent = NULL, char const * name = NULL);
	virtual ~PrintDialog();
	
	char const * getFrom() {
		return line_from->text();
	}

	char const * getTo() {
		return line_to->text();
	}

	PrinterParams::Target getTarget() {
		if (radio_toprinter->isChecked())
			return PrinterParams::PRINTER;
		else
			return PrinterParams::FILE;
	}

	char const * getPrinter() {
		return line_printername->text();
	}

	char const * getFile() {
		return line_filename->text();
	}
	
	PrinterParams::WhichPages getWhichPages() {
		if (radio_oddpages->isChecked())
			return PrinterParams::ODD;
		else if (radio_evenpages->isChecked())
			return PrinterParams::EVEN;
		else
			return PrinterParams::ALL;
	}

	bool getReverse() {
		return check_reverse->isChecked();
	}

	bool getSort() {
		return check_sort->isChecked();
	}

	char const * getCount() {
		return line_count->text();
	}

	void setTarget(PrinterParams::Target t) {
		radio_toprinter->setChecked(t == PrinterParams::PRINTER);
		radio_tofile->setChecked(t != PrinterParams::PRINTER);
	}
	 
       	void setPrinter(char const * name) {
		line_printername->setText(name);
	} 
 
	void setFile(char const * name) {
		line_filename->setText(name);
	}	 
 
	void setWhichPages(PrinterParams::WhichPages wp) {
		switch (wp) {
			case PrinterParams::ALL:
				radio_allpages->setChecked(true);
				break;
			case PrinterParams::EVEN:
				radio_evenpages->setChecked(true);
				break;
			case PrinterParams::ODD:
				radio_oddpages->setChecked(true);
				break;
		}
	}
 
	void setReverse(bool on) {
		check_reverse->setChecked(on);
	}
 
	void setSort(bool on) {
		check_sort->setChecked(on);
	}
	 
	void setCount(int num) {
		line_count->setText(tostr(num).c_str());
		check_sort->setEnabled(num > 1);
	}

	void setFrom(char const * text) {
		line_from->setText(text);
	}

	void setTo(char const * text) {
		line_to->setText(text);
	}

protected slots:
 
	void clickedCancel() {
		form_->CancelButton(); 
	}
 
	void clickedPrint() {
		form_->OKButton();
	}

	/// open up the browser to select ps file 
	void clickedBrowse();
	
	/// validate and change collate status
	void changedCount(char const * text);
 
private:
    	FormPrint * form_;
};
#endif // PRINTDLG_H
