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

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class QMathDialog;

class QMath {
public:
	friend class QMathDialog;

	QMath();

	/// temporary
	void do_show();

	/// build the dialog (should be private)
	virtual void build_dialog();

	/// insert a math symbol into the doc
	void insert(string const & name);

	/// insert a cube root
	void insertCubeRoot();

	/// insert a matrix
	void insertMatrix();

	/// insert delim
	void insertDelim(string const & str);

	/// add a subscript
	void subscript();

	/// add a superscript
	void superscript();

	/// switch between display and inline
	void toggleDisplay();
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents() {}

	// FIXME: temp
	QMathDialog * dialog_;
};

#endif // QMATH_H
