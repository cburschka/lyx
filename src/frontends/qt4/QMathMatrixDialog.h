// -*- C++ -*-
/**
 * \file QMathMatrixDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QMATHMATRIXDIALOG_H
#define QMATHMATRIXDIALOG_H

#include "QDialogView.h"

#include "ui_MathMatrixUi.h"

#include <QDialog>

namespace lyx {
namespace frontend {

class QMathMatrix;
class ControlMath;


class QMathMatrixDialog : public QDialog, public Ui::QMathMatrixUi {
	Q_OBJECT
public:
	QMathMatrixDialog(QMathMatrix * form);
public Q_SLOTS:
	void slotOK();
	void slotClose();
protected Q_SLOTS:
	virtual void columnsChanged(int);
	virtual void rowsChanged(int);
	virtual void change_adaptor();
private:
	QMathMatrix * form_;
};


class QMathMatrix : public QController<ControlMath, QView<QMathMatrixDialog> > {
public:
	friend class QMathMatrixDialog;

	QMathMatrix(Dialog &);

private:
	virtual void apply() {}
	virtual void update_contents() {}
	/// Build the dialog.
	virtual void build_dialog();
};


} // namespace frontend
} // namespace lyx

#endif // QMATHMATRIXDIALOG_H
