// -*- C++ -*-
/**
 * \file QMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QMATH_H
#define QMATH_H


#include "QDialogView.h"

class ControlMath;
class QMathDialog;
class QMathMatrixDialog;
class QDelimiterDialog;

class QMath : public QController<ControlMath, QView<QMathDialog> > {
public:
	friend class QMathDialog;

	QMath(Dialog &);

private:
	virtual void apply() {}
	virtual void update_contents() {}
	/// Build the dialog.
	virtual void build_dialog();
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


class QMathDelimiter : public QController<ControlMath, QView<QDelimiterDialog> > {
public:
	friend class QDelimiterDialog;

	QMathDelimiter(Dialog &);

private:
	virtual void apply() {}
	virtual void update_contents() {}
	/// Build the dialog.
	virtual void build_dialog();
};

#endif // QMATH_H
