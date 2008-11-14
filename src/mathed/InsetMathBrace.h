// -*- C++ -*-
/**
 * \file InsetMathBrace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_BRACEINSET_H
#define MATH_BRACEINSET_H

#include "InsetMathNest.h"


namespace lyx {


/// Extra nesting
class InsetMathBrace : public InsetMathNest {
public:
	///
	InsetMathBrace();
	///
	InsetMathBrace(MathData const & ar);
	/// identifies brace insets
	InsetMathBrace * asBraceInset() { return this; }
	/// identifies brace insets
	InsetMathBrace const * asBraceInset() const { return this; }
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void mathmlize(MathStream &) const;
	///
	void infoize(odocstream & os) const;
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
