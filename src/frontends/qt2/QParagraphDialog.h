// -*- C++ -*-
/**
 * \file QParagraphDialog.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef QPARAGRAPHDIALOG_H
#define QPARAGRAPHDIALOG_H

#include <config.h>

#include "QParagraph.h"
#include "ui/QParagraphDialogBase.h"

#include <qevent.h>

class QParagraphDialog : public QParagraphDialogBase
{
	Q_OBJECT
public:
	QParagraphDialog(QParagraph * form);

protected:
	void closeEvent (QCloseEvent * e);
private:
	QParagraph * form_;
	
protected slots:
	void change_adaptor();
	void enableAbove(int);
	void enableBelow(int);
	void enableLinespacingValue(int);
};

#endif // QPARAGRAPHDIALOG_H
