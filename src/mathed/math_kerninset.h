// -*- C++ -*-
#ifndef MATH_CHEATINSET_H
#define MATH_CHEATINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/// Some hack for visual effects

class MathKernInset : public MathNestInset {
public:
	///
	MathKernInset();
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void metrics(MathMetricsInfo const &cwmist) const;
};
#endif
