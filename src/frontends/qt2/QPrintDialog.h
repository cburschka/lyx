/**
 * \file QPrintDialog.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#include <config.h>
 
#include "ui/QPrintDialogBase.h"
#include "lyxrc.h" 
#include "PrinterParams.h"

class QPrint;

class QPrintDialog : public QPrintDialogBase
{ Q_OBJECT

public:
	QPrintDialog(QPrint * f, QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags fl = 0);
	~QPrintDialog();
	
	// FIXME: these should return std::string !
	const char * getFrom();
	const char * getTo();
	const char * getPrinter();
	const char * getFile();
	const char * getCount();
	PrinterParams::Target getTarget();
	PrinterParams::WhichPages getWhichPages();
	bool getReverse();
	bool getSort();
	void setFrom(const char *);
	void setTo(const char *);
	void setPrinter(const char *);
	void setFile(const char *);
	void setCount(int);
	void setTarget(PrinterParams::Target);
	void setWhichPages(PrinterParams::WhichPages);
	void setReverse(bool);
	void setSort(bool);
	  

protected slots:
	void cancel_adaptor();
	void browse_file();
	void print();
	void set_collate(int);

private:
	QPrint * form_;

};

#endif // QPRINTDIALOG_H
