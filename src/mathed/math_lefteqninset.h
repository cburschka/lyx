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
	string name() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
};
#endif
