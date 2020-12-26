// -*- C++ -*-
/**
 * \file InsetMathString.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_STRINGINSET_H
#define MATH_STRINGINSET_H

#include "InsetMath.h"

#include "support/docstring.h"


namespace lyx {

/** Some collection of chars with similar properties
 *  mainly for math-extern
 */

class InsetMathString : public InsetMath {
public:
	///
	explicit InsetMathString(docstring const & s);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	docstring str() const { return str_; }
	///
	InsetMathString * asStringInset() override { return this; }
	///
	InsetMathString const * asStringInset() const override { return this; }

	///
	void normalize(NormalStream &) const override;
	///
	void octave(OctaveStream &) const override;
	///
	void maple(MapleStream &) const override;
	///
	void mathematica(MathematicaStream &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void write(WriteStream & os) const override;
	///
	InsetCode lyxCode() const override { return MATH_STRING_CODE; }

private:
	Inset * clone() const override;
	/// the string
	docstring str_;
};

} // namespace lyx

#endif
