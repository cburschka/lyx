// -*- C++ -*-
/**
 * \file formula.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FORMULA_H
#define INSET_FORMULA_H

#include "math_hullinset.h"


/// The main LyX math inset
class InsetFormula : public MathHullInset {
public:
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
};

// We don't really want to mess around with mathed stuff outside mathed.
// So do it here.
void mathDispatch(LCursor & cur, FuncRequest const & cmd);

#endif
