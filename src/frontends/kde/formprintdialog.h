/*
 * formprintdialog.h
 * (C) 2000 LyX Team
 * John Levon, moz@compsoc.man.ac.uk
 */ 
 
#ifndef FORMPRINTDIALOG_H
#define FORMPRINTDIALOG_H

#include "formprintdialogdata.h"
#include "lyxrc.h" 
#include "PrinterParams.h"
#include "FormPrint.h"

class FormPrintDialog : public FormPrintDialogData
{
	Q_OBJECT
public:

	FormPrintDialog(FormPrint *f, QWidget* parent = NULL, const char* name = NULL);
	virtual ~FormPrintDialog();
	
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
	 
	void setCount(const char *num) {
		count->setText(num);
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

	void clickedBrowse();
 
private:
    	FormPrint *form_;
};
#endif // FORMPRINTDIALOG_H
