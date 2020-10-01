// -*- C++ -*-
/**
 * \file InsetMathSqrt.h
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

#include "InsetMathNest.h"


namespace lyx {


/// \c InsetMathSqrt The square root inset.
class InsetMathSqrt : public InsetMathNest {
public:
	///
	explicit InsetMathSqrt(Buffer * buf);
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter &, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;

	///
	void write(WriteStream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	InsetCode lyxCode() const override { return MATH_SQRT_CODE; }
	///
	void validate(LaTeXFeatures &) const override;

private:
	Inset * clone() const override;
};


} // namespace lyx
#endif
