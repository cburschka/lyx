/**
 * \file QFloatDialog.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#ifndef QFLOATDIALOG_H
#define QFLOATDIALOG_H

#include <config.h>

#include "ui/QFloatDialogBase.h"

class QFloat;

class QFloatDialog : public QFloatDialogBase
{ Q_OBJECT

public:
	QFloatDialog(QFloat * form);

protected slots:
	virtual void change_adaptor();
	virtual void tbhpClicked();
	virtual void heredefinitelyClicked();
	virtual void spanClicked();
 
protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	QFloat * form_;
};

#endif // QFLOATDIALOG_H
