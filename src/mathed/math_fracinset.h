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
	void metrics(MathMetricsInfo const & mi) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void metrics(TextMetricsInfo const & mi) const;
	///
	void draw(TextPainter &, int x, int y) const;
	///
	MathFracInset * asFracInset();

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
public:
	///
	const bool atop_;
};

#endif
