// -*- C++ -*-
#ifndef MATH_STRINGINSET_H
#define MATH_STRINGINSET_H

#include "math_diminset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Some cllection of chars with similar properties
    \author André Pönitz
 */

class MathStringInset : public MathDimInset {
public:
	///
	MathStringInset();
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
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;

public:
	/// the string
	string str_;
};
#endif
