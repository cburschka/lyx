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
	marker_type marker(BufferView const *) const override { return marker_type::MARKER2; }
	///
	bool idxUpDown(Cursor &, bool up) const override;
	///
	bool idxBackward(Cursor &) const override { return false; }
	///
	bool idxForward(Cursor &) const override { return false; }
	///
	InsetMathFracBase * asFracBaseInset() override { return this; }
	///
	InsetMathFracBase const * asFracBaseInset() const override { return this; }
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
	bool idxForward(Cursor &) const override;
	///
	bool idxBackward(Cursor &) const override;
	///
	MathClass mathClass() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter &, int x, int y) const override;
	/// identifies FracInsets
	InsetMathFrac * asFracInset() override;
	/// identifies FracInsets
	InsetMathFrac const * asFracInset() const override;
	///
	docstring name() const override;
	///
	bool extraBraces() const override;
	///
	void write(WriteStream & os) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
private:
	/// vertical displacement
	int dy(FontInfo & fi) const;
	///
	Inset * clone() const override;
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
	void write(WriteStream & os) const override;
	///
	void normalize(NormalStream &) const override;
	/// Generalized fractions are of inner class (see The TeXbook, p.292)
	MathClass mathClass() const override { return MC_INNER; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	bool extraBraces() const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_FRAC_CODE; }
private:
	Inset * clone() const override;
	///
	int dw(int height) const;
	///
	Kind kind_;
};



} // namespace lyx

#endif
