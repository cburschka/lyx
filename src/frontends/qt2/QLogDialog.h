// -*- C++ -*-
/**
 * \file QLogDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QLOGDIALOG_H
#define QLOGDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QLogDialogBase.h"

class QLog;

class QLogDialog : public QLogDialogBase
{ Q_OBJECT

public:
	QLogDialog(QLog * form);

protected slots:
	virtual void updateClicked();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QLog * form_;
};

#endif // QLOGDIALOG_H
