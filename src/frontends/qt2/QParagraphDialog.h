// -*- C++ -*-
/**
 * \file QParagraphDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPARAGRAPHDIALOG_H
#define QPARAGRAPHDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QParagraphDialogBase.h"

class QParagraph;

class QParagraphDialog : public QParagraphDialogBase {
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
