// -*- C++ -*-
#ifndef MATH_LEFTEQNINSET_H
#define MATH_LEFTEQNINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/// The \kern primitive

class MathLefteqnInset : public MathNestInset {
public:
	///
	MathLefteqnInset();
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
};
#endif
