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
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	/// identifies FracInsets
	MathFracInset * asFracInset();
	/// identifies FracInsets
	MathFracInset const * asFracInset() const;
	///
	string name() const;

	///
	void write(WriteStream & os) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathematicize(MathematicaStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void mathmlize(MathMLStream &) const;
public:
	///
	const bool atop_;
};

#endif
