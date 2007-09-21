// -*- C++ -*-
/**
 * \file InsetMathFrac.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FRACINSET_H
#define MATH_FRACINSET_H

#include "InsetMathFracBase.h"


namespace lyx {


/// Fraction like objects (frac, binom)
class InsetMathFrac : public InsetMathFracBase {
public:
	///
	enum Kind {
		FRAC,
		OVER,
		ATOP,
		NICEFRAC,
		UNITFRAC,
		UNIT
	};

	///
	explicit InsetMathFrac(Kind kind = FRAC, idx_type ncells = 2);
	///
	bool idxRight(Cursor &) const;
	///
	bool idxLeft(Cursor &) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	/// identifies FracInsets
	InsetMathFrac * asFracInset();
	/// identifies FracInsets
	InsetMathFrac const * asFracInset() const;
	///
	docstring name() const;
	///
	bool extraBraces() const;

	///
	void write(WriteStream & os) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
public:
	virtual Inset * clone() const;
	///
	Kind kind_;
};



} // namespace lyx
#endif
