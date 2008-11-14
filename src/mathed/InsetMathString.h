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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	docstring str() const { return str_; }
	///
	InsetMathString * asStringInset() { return this; }
	///
	InsetMathString const * asStringInset() const { return this; }

	///
	void normalize(NormalStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void write(WriteStream & os) const;

private:
	virtual Inset * clone() const;
	/// the string
	docstring str_;
};

} // namespace lyx

#endif
