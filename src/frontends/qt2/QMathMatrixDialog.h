// -*- C++ -*-
/**
 * \file QMathMatrixDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Juergen Spitzmueller
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QMATHMATRIXDIALOG_H
#define QMATHMATRIXDIALOG_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/QMathMatrixDialogBase.h"

class QMath;

class QMathMatrixDialog : public QMathMatrixDialogBase {
	Q_OBJECT
public:
	QMathMatrixDialog(QMath * form);
public slots:
	void slotOK();
	void slotClose();
protected slots:
	virtual void columnsChanged(int);
	virtual void rowsChanged(int);
	virtual void change_adaptor();
private:
	QMath * form_;
};

#endif // QMATHMATRIXDIALOG_H
