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
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(NormalStream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
};
#endif
