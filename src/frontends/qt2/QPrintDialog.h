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
