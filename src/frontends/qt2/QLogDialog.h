/**
 * \file QLogDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QLOGDIALOG_H
#define QLOGDIALOG_H
 
#include <config.h>
 
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
