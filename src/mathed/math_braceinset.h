// -*- C++ -*-
#ifndef MATH_BRACEINSET_H
#define MATH_BRACEINSET_H

#include "math_nestinset.h"


/** Extra nesting
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
*/

class MathBraceInset : public MathNestInset {
public:
	///
	MathBraceInset();
	///
	MathBraceInset(MathArray const & ar);
	///
	InsetBase * clone() const;
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
	void mathmlize(MathMLStream &) const;
	///
	void infoize(std::ostream & os) const;

private:
	/// width of brace character
	mutable int wid_;
};

#endif
