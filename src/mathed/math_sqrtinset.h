// -*- C++ -*-
#ifndef MATH_SQRTINSET_H
#define MATH_SQRTINSET_H

#include "math_nestinset.h"


/** The square root inset.
 *  \author Alejandro Aguilar Siearra
 *
 * Full author contact details are available in file CREDITS
 */
class MathSqrtInset : public MathNestInset {
public:
	///
	MathSqrtInset();
	///
	MathInset * clone() const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	Dimension metrics(MetricsInfo & mi) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void mathmlize(MathMLStream &) const;
};
#endif
