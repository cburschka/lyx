// -*- C++ -*-
/**
 * \file QFloatDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QFLOATDIALOG_H
#define QFLOATDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QFloatDialogBase.h"

class QFloat;

class QFloatDialog : public QFloatDialogBase {
	Q_OBJECT
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
