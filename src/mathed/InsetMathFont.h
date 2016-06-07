// -*- C++ -*-
/**
 * \file InsetMathFont.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_FONTINSET_H
#define MATH_FONTINSET_H

#include "InsetMathNest.h"


namespace lyx {


class latexkeys;

/// Inset for font changes
class InsetMathFont : public InsetMathNest {
public:
	///
	explicit InsetMathFont(Buffer * buf, latexkeys const * key);
	///
	InsetMathFont * asFontInset() { return this; }
	///
	InsetMathFont const * asFontInset() const { return this; }
	/// are we in math mode, text mode, or unsure?
	mode_type currentMode() const;
	/// do we allow changing mode during latex export?
	bool lockedMode() const;
	///
	docstring name() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter & pi, int x, int y) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void mathmlize(MathStream &) const;
	///
	void htmlize(HtmlStream &) const;
	///
	void infoize(odocstream & os) const;
	///
	int kerning(BufferView const * bv) const { return cell(0).kerning(bv); }
	///
	InsetCode lyxCode() const { return MATH_FONT_CODE; }

private:
	std::string font() const;
	///
	virtual Inset * clone() const;
	/// the font to be used on screen
	latexkeys const * key_;
};


} // namespace lyx
#endif
