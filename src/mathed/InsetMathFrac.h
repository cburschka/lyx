// -*- C++ -*-
/**
 * \file InsetMathFrac.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FRAC_H
#define MATH_FRAC_H

#include "InsetMathNest.h"


namespace lyx {


class InsetMathFracBase : public InsetMathNest {
public:
	///
	InsetMathFracBase(Buffer * buf, idx_type ncells = 2);
	///
	bool idxUpDown(Cursor &, bool up) const;
	///
	bool idxBackward(Cursor &) const { return false; }
	///
	bool idxForward(Cursor &) const { return false; }
	///
	InsetMathFracBase * asFracBaseInset() { return this; }
	///
	InsetMathFracBase const * asFracBaseInset() const { return this; }
};



/// Fraction like objects (frac, binom)
class InsetMathFrac : public InsetMathFracBase {
public:
	///
	enum Kind {
		FRAC,
		CFRAC,
		CFRACLEFT,
		CFRACRIGHT,
		DFRAC,
		TFRAC,
		OVER,
		ATOP,
		NICEFRAC,
		UNITFRAC,
		UNIT
	};
	///
	explicit InsetMathFrac(Buffer * buf, Kind kind = FRAC, idx_type ncells = 2);
	///
	bool idxForward(Cursor &) const;
	///
	bool idxBackward(Cursor &) const;
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
	void htmlize(HtmlStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
public:
	Inset * clone() const;
	///
	Kind kind_;
};



/// Binom like objects
class InsetMathBinom : public InsetMathFracBase {
public:
	///
	enum Kind {
		BINOM,
		DBINOM,
		TBINOM,
		CHOOSE,
		BRACE,
		BRACK
	};
	///
	explicit InsetMathBinom(Buffer * buf, Kind kind = BINOM);
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	/// draw decorations.
	void drawDecoration(PainterInfo & pi, int x, int y) const
	{ drawMarkers2(pi, x, y); }
	///
	bool extraBraces() const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_FRAC_CODE; }
private:
	Inset * clone() const;
	///
	int dw(int height) const;
	///
	Kind kind_;
};



} // namespace lyx

#endif
