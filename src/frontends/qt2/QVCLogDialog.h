// -*- C++ -*-
/**
 * \file QVCLogDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QVCLOGDIALOG_H
#define QVCLOGDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QVCLogDialogBase.h"

class QVCLog;

class QVCLogDialog : public QVCLogDialogBase {
	Q_OBJECT
public:
	QVCLogDialog(QVCLog * form);
protected slots:
	virtual void updateClicked();
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QVCLog * form_;
};

#endif // QVCLOGDIALOG_H
