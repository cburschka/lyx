// -*- C++ -*-
/**
 * \file InsetMathSpecialChar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_SPECIALCHARINSET_H
#define MATH_SPECIALCHARINSET_H

#include "InsetMath.h"

#include "support/docstring.h"

namespace lyx {

/// The special character inset.
class InsetMathSpecialChar : public InsetMath
{
public:
	///
	explicit InsetMathSpecialChar(docstring const & name);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter &, int x, int y) const override;
	///
	int kerning(BufferView const *) const override { return kerning_; }
	///
	void write(WriteStream & os) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void normalize(NormalStream & ns) const override;
	///
	void octave(OctaveStream & os) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void mathmlize(MathStream & ms) const override;
	///
	void htmlize(HtmlStream & ms) const override;
	/// identifies SpecialChar insets
	InsetMathSpecialChar const * asSpecialCharInset() const override { return this; }
	///
	docstring name() const override { return name_; }
	///
	char_type getChar() const override { return char_; }
	///
	InsetCode lyxCode() const override { return MATH_SPECIALCHAR_CODE; }

private:
	Inset * clone() const override;
	/// the latex name
	docstring name_;
	/// the displayed character
	char_type char_;
	/// cached kerning for superscript
	mutable int kerning_;
};

} // namespace lyx

#endif // MATH_SPECIALCHARINSET_H
