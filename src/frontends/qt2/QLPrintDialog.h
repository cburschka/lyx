// -*- C++ -*-
/**
 * \file QLPrintDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLPRINTDIALOG_H
#define QLPRINTDIALOG_H


#include "ui/QPrintDialogBase.h"

class QPrint;

class QLPrintDialog : public QPrintDialogBase {
	Q_OBJECT
public:
	QLPrintDialog(QPrint * f);
protected slots:
	virtual void change_adaptor();
	virtual void browseClicked();
	virtual void fileChanged();
	virtual void copiesChanged(int);
	virtual void printerChanged();
	virtual void pagerangeChanged();
private:
	QPrint * form_;
};

#endif // QLPRINTDIALOG_H
