// -*- C++ -*-
/**
 * \file QPrintDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPRINTDIALOG_H
#define QPRINTDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QPrintDialogBase.h"

class QPrint;

class QPrintDialog : public QPrintDialogBase {
	Q_OBJECT
public:
	QPrintDialog(QPrint * f);
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

#endif // QPRINTDIALOG_H
