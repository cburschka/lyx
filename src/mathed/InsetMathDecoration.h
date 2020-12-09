// -*- C++ -*-
/**
 * \file InsetMathDecoration.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_DECORATIONINSET_H
#define MATH_DECORATIONINSET_H

#include "InsetMathNest.h"


namespace lyx {

class latexkeys;

/// Decorations and accents over (below) a math object
class InsetMathDecoration : public InsetMathNest {
public:
	///
	explicit InsetMathDecoration(Buffer * buf, latexkeys const * key);
	///
	mode_type currentMode() const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	void write(WriteStream & os) const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void normalize(NormalStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	MathClass mathClass() const override;
	/// The default limits value in \c display style
	Limits defaultLimits(bool display) const override;
	/// whether the inset has limit-like sub/superscript
	Limits limits() const override { return limits_; }
	/// sets types of sub/superscripts
	void limits(Limits lim) override { limits_ = lim; }
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return MATH_DECORATION_CODE; }
	///
	void mathmlize(MathStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
private:
	Inset * clone() const override;
	///
	bool upper() const;
	///
	bool protect() const;
	/// is it a wide decoration?
	bool wide() const;

	///
	latexkeys const * key_;
	///
	Limits limits_ = AUTO_LIMITS;
	// FIXME: this should depend on BufferView
	/// height cache of deco
	mutable int dh_ = 0;
	/// vertical offset cache of deco
	mutable int dy_ = 0;
	/// width for non-wide deco
	mutable int dw_ = 0;
};

} // namespace lyx

#endif
