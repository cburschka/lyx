// -*- C++ -*-
#ifndef MATH_FRACINSET_H
#define MATH_FRACINSET_H

#include "math_fracbase.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Fraction like objects (frac, binom)
    \author Alejandro Aguilar Sierra
 */
class MathFracInset : public MathFracbaseInset {
public:
	///
	explicit MathFracInset(bool atop = false);
	///
	MathInset * clone() const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	string maplize() const;
public:
	///
	const bool atop_;
};

#endif
