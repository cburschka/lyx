// -*- C++ -*-
#ifndef MATH_SQRTINSET_H
#define MATH_SQRTINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** The square root inset.
    \author Alejandro Aguilar Siearra
 */
class MathSqrtInset : public MathNestInset {
public:
	///
	MathSqrtInset();
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
};
#endif
