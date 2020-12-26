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
	InsetMathFont * asFontInset() override { return this; }
	///
	InsetMathFont const * asFontInset() const override { return this; }
	/// are we in math mode, text mode, or unsure?
	mode_type currentMode() const override;
	/// do we allow changing mode during latex export?
	bool lockedMode() const override;
	///
	void write(WriteStream & os) const override;
	///
	docstring name() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter & pi, int x, int y) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void infoize(odocstream & os) const override;
	///
	int kerning(BufferView const * bv) const override { return cell(0).kerning(bv); }
	///
	InsetCode lyxCode() const override { return MATH_FONT_CODE; }

private:
	std::string font() const;
	///
	Inset * clone() const override;
	/// the font to be used on screen
	latexkeys const * key_;
};


} // namespace lyx
#endif
