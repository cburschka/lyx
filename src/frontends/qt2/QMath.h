// -*- C++ -*-
/**
 * \file QMath.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QMATH_H
#define QMATH_H

#include <config.h>

#include "LString.h"
 
#ifdef __GNUG__
#pragma interface
#endif

class QMath
{
public:
	friend class QMathDialog;

	QMath();

	/// temporary
	void do_show();
 
	/// build the dialog (should be private)
	virtual void build_dialog();

	/// insert a math symbol into the doc
	void insert_symbol(string const & name);
 
	/// add a subscript
	void subscript();

	/// add a superscript
	void superscript();
 
private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents() {};

	// FIXME: temp 
	QMathDialog * dialog_; 
};

#endif // QMATH_H
