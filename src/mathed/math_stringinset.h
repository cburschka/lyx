// -*- C++ -*-
#ifndef MATH_STRINGINSET_H
#define MATH_STRINGINSET_H

#include "math_diminset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Some collection of chars with similar properties
 *  maily for math-extern
 *  \author André Pönitz
 */

class MathStringInset : public MathDimInset {
public:
	///
	explicit MathStringInset(string const & s);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	string str() const { return str_; }
	///
	MathStringInset * asStringInset() { return this; }

	///
	void normalize(NormalStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathematicize(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;

private:
	/// the string
	string str_;
};
#endif
