// -*- C++ -*-
/**
 * \file math_sqrtinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SQRTINSET_H
#define MATH_SQRTINSET_H

#include "math_nestinset.h"


/// \c MathSqrtInset The square root inset.
class MathSqrtInset : public MathNestInset {
public:
	///
	MathSqrtInset();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
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
