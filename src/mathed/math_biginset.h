// -*- C++ -*-

/** 
 *  \file math_biginset.h
 *
 *  This file is part of LyX, the document processor.
 *  Licence details can be found in the file COPYING.
 *
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BIGINSET_H
#define MATH_BIGINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_diminset.h"
#include "LString.h"

/// Inset for \bigl & Co.
class MathBigInset : public MathDimInset {
public:
	///
	MathBigInset(string const & name, string const & delim);
	///
	MathInset * clone() const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void normalize(NormalStream & os) const;

private:
	///
	size_type size() const;
	///
	double increase() const;

	/// \bigl or what?
	string const name_;
	/// ( or [ or Vert...
	string const delim_;
};

#endif
