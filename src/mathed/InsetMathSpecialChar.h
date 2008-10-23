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

namespace lyx {

/// The special character inset.
class InsetMathSpecialChar : public InsetMath {
public:
	///
	explicit InsetMathSpecialChar(docstring name);
	///
	void setBuffer(Buffer &) {}
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
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream & ms) const;
	/// identifies SpecialChar insets
	InsetMathSpecialChar const * asSpecialCharInset() const { return this; }
	///
	docstring name() const { return name_; }
	///
	char_type getChar() const { return char_; }

private:
	virtual Inset * clone() const;
	/// the latex name
	docstring name_;
	/// the displayed character
	char_type char_;
	/// cached kerning for superscript
	mutable int kerning_;
};

} // namespace lyx

#endif
