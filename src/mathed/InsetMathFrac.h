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

#ifndef MATH_FRAC_H
#define MATH_FRAC_H

#include "InsetMathNest.h"


namespace lyx {


class InsetMathFracBase : public InsetMathNest {
public:
	///
	explicit InsetMathFracBase(idx_type ncells = 2);
	///
	bool idxUpDown(Cursor &, bool up) const;
	///
	bool idxBackward(Cursor &) const { return false; }
	///
	bool idxForward(Cursor &) const { return false; }
};



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
	void validate(LaTeXFeatures & features) const;
public:
	Inset * clone() const;
	///
	Kind kind_;
};


/// \dfrac support
class InsetMathDFrac : public InsetMathFrac {
public:
	///
	InsetMathDFrac() {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	docstring name() const;
	///
	void mathmlize(MathStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	Inset * clone() const;
};


/// \tfrac support
class InsetMathTFrac : public InsetMathFrac {
public:
	///
	InsetMathTFrac() {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	docstring name() const;
	///
	void mathmlize(MathStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	Inset * clone() const;
};


/// \cfrac support
class InsetMathCFrac : public InsetMathFrac {
public:
	///
	InsetMathCFrac() {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	docstring name() const;
	///
	void mathmlize(MathStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	Inset * clone() const;
};


/// Binom like objects
class InsetMathBinom : public InsetMathFracBase {
public:
	///
	enum Kind {
		BINOM,
		CHOOSE,
		BRACE,
		BRACK
	};

	///
	explicit InsetMathBinom(Kind kind = BINOM);
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
	void validate(LaTeXFeatures & features) const;
private:
	Inset * clone() const;
	///
	int dw(int height) const;
	///
	Kind kind_;
};


/// \dbinom support
class InsetMathDBinom : public InsetMathFracBase {
public:
	///
	InsetMathDBinom() {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	docstring name() const;
	///
	void mathmlize(MathStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	Inset * clone() const;
	///
	int dw(int height) const;
};


/// \tbinom support
class InsetMathTBinom : public InsetMathFracBase {
public:
	///
	InsetMathTBinom() {}
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	docstring name() const;
	///
	void mathmlize(MathStream &) const;
	///
	void validate(LaTeXFeatures & features) const;
private:
	Inset * clone() const;
	///
	int dw(int height) const;
};


} // namespace lyx

#endif
