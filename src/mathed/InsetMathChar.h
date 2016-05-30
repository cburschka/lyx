// -*- C++ -*-
/**
 * \file InsetMathChar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_CHARINSET_H
#define MATH_CHARINSET_H

#include "InsetMath.h"

namespace lyx {

class latexkeys;

/// The base character inset.
class InsetMathChar : public InsetMath {
public:
	///
	explicit InsetMathChar(char_type c);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	int kerning(BufferView const *) const { return kerning_; }

	///
	void write(WriteStream & os) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void octave(OctaveStream & os) const;
	///
	void mathmlize(MathStream & ms) const;
	///
	void htmlize(HtmlStream & ms) const;
	/// identifies Charinsets
	InsetMathChar const * asCharInset() const { return this; }
	///
	char_type getChar() const { return char_; }
	///
	MathClass mathClass() const;
	///
	InsetCode lyxCode() const { return MATH_CHAR_CODE; }

private:
	virtual Inset * clone() const;
	/// the character
	char_type const char_;
	/// cached kerning for superscript
	mutable int kerning_;
	/// Inset to substitute char for, for on-screen display in math mode, as
	/// performed by LaTeX (#9893):
	/// * -> \ast (U+2217)
	/// - -> \lyxminus (U+2212)
	/// : -> \ordinarycolon (U+2236)
	///
	/// For cosmetic reasons, +, >, <, and = are also substituted to force the
	/// use of CM fonts for uniformity. If CM fonts are replaced with unicode
	/// math fonts, this should be removed, and substitutions of "'", ",", and
	/// ";" added.
	///
	/// Null if there is no substitute.
	latexkeys const * const subst_;
};

} // namespace lyx

#endif
