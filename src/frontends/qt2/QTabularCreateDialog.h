/**
 * \file QTabularCreateDialog.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#ifndef QTABULARCREATEDIALOG_H
#define QTABULARCREATEDIALOG_H

#include "ui/QTabularCreateDialogBase.h"

class QTabularCreate;

class QTabularCreateDialog : public QTabularCreateDialogBase
{ Q_OBJECT

public:
	QTabularCreateDialog(QTabularCreate * form);

protected slots:
	virtual void columnsChanged(int);
	virtual void rowsChanged(int);

private:
	QTabularCreate * form_;
};

#endif // QTABULARCREATEDIALOG_H
