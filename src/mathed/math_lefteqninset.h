// -*- C++ -*-
#ifndef MATH_LEFTEQNINSET_H
#define MATH_LEFTEQNINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/// Support for LaTeX's \\lefteqn command

class MathLefteqnInset : public MathNestInset {
public:
	///
	MathLefteqnInset();
	///
	MathInset * clone() const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MathMetricsInfo & st) const;
};
#endif
