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
	mode_type currentMode() const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void normalize(NormalStream & os) const;
	///
	void infoize(odocstream & os) const;
	///
	MathClass mathClass() const;
	///
	bool isScriptable() const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return MATH_DECORATION_CODE; }
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
private:
	virtual Inset * clone() const;
	///
	bool upper() const;
	///
	bool protect() const;
	/// is it a wide decoration?
	bool wide() const;

	///
	latexkeys const * key_;
	/// height cache of deco
	mutable int dh_;
	/// vertical offset cache of deco
	mutable int dy_;
	/// width for non-wide deco
	mutable int dw_;
};

} // namespace lyx

#endif
