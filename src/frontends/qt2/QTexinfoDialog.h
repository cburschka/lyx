/**
 * \file QTexinfoDialog.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#ifndef QTEXINFODIALOG_H
#define QTEXINFODIALOG_H
 
#include <config.h>
 
#include "ui/QTexinfoDialogBase.h"

class QTexinfo;

class QTexinfoDialog : public QTexinfoDialogBase
{ Q_OBJECT

public:
	QTexinfoDialog(QTexinfo * form);

protected slots:
	virtual void change_adaptor();
	virtual void helpClicked();
	virtual void rescanClicked();
	virtual void viewClicked();
	virtual void update();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QTexinfo * form_;
};

#endif // QTEXINFODIALOG_H
