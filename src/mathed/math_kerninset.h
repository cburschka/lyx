// -*- C++ -*-
#ifndef MATH_CHEATINSET_H
#define MATH_CHEATINSET_H

#include "math_diminset.h"
#include "vspace.h"
#include "LString.h"
#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/// The \kern primitive
/// Some hack for visual effects

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
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void metrics(MathMetricsInfo & st) const;
private:
	/// width in em
	LyXLength wid_;
};
#endif
