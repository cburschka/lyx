// -*- C++ -*-
#ifndef MATH_CHEATINSET_H
#define MATH_CHEATINSET_H

#include "math_diminset.h"
#include "vspace.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/// The \kern primitive

class MathKernInset : public MathDimInset {
public:
	///
	MathKernInset();
	///
	explicit MathKernInset(LyXLength const & wid);
	///
	explicit MathKernInset(string const & wid);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(NormalStream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
private:
	/// width in em
	LyXLength wid_;
};
#endif
