/**
 * \file QVCLogDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QVCLOGDIALOG_H
#define QVCLOGDIALOG_H
 
#include <config.h>
 
#include "ui/QVCLogDialogBase.h"

class QVCLog;

class QVCLogDialog : public QVCLogDialogBase
{ Q_OBJECT

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
