/*
 * printdlg.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */ 
 
#ifndef PRINTDLG_H
#define PRINTDLG_H


#include "printdlgdata.h"
#include "support/lstrings.h"
#include "lyxrc.h" 
#include "PrinterParams.h"
#include "FormPrint.h"

class PrintDialog : public PrintDialogData
{
	Q_OBJECT
public:

	PrintDialog(FormPrint *f, QWidget* parent = NULL, const char* name = NULL);
	virtual ~PrintDialog();
	
	const char *getFrom() {
		return from->text();
	}

	const char *getTo() {
		return to->text();
	}

	PrinterParams::Target getTarget() {
		if (toprinter->isChecked())
			return PrinterParams::PRINTER;
		else
			return PrinterParams::FILE;
	}

	const char *getPrinter() {
		return printername->text();
	}

	const char *getFile() {
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

	const char *getCount() {
		return count->text();
	}

	void setTarget(PrinterParams::Target t) {
		toprinter->setChecked(t==PrinterParams::PRINTER);
		tofile->setChecked(t!=PrinterParams::PRINTER);
	}
	 
       	void setPrinter(const char *name) {
		printername->setText(name);
	} 
 
	void setFile(const char *name) {
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

	void setFrom(const char *text) {
		from->setText(text);
	}

	void setTo(const char *text) {
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
	void changedCount(const char *text);
 
private:
    	FormPrint *form_;
};
#endif // PRINTDLG_H
