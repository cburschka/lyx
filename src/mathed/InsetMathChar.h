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
	bool isMathBin() const;
	///
	bool isMathRel() const;
	///
	bool isMathPunct() const;
	///
	InsetCode lyxCode() const { return MATH_CHAR_CODE; }

private:
	virtual Inset * clone() const;
	/// the character
	char_type char_;
	/// cached kerning for superscript
	mutable int kerning_;
};

} // namespace lyx

#endif
