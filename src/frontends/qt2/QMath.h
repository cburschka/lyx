// -*- C++ -*-
/**
 * \file QMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QMATH_H
#define QMATH_H


#include "QDialogView.h"

class ControlMath2;
class QMathDialog;

class QMath : public QController<ControlMath2, QView<QMathDialog> > {
public:
	friend class QMathDialog;

	QMath(Dialog &);

private:
	virtual void apply() {}
	virtual void update_contents() {}
	/// Build the dialog.
	virtual void build_dialog();
};

#endif // QMATH_H
