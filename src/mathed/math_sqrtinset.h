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
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathematicize(MathematicaStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void mathmlize(MathMLStream &) const;
};
#endif
