// -*- C++ -*-
#ifndef MATH_STRINGINSET_H
#define MATH_STRINGINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Some cllection of chars with similar properties
    \author André Pönitz
 */

class MathStringInset : public MathInset {
public:
	///
	MathStringInset(string const & s, MathTextCodes t);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;
	/// 
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;
	///
	void octavize(OctaveStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;

private:
	/// the string
	string str_;
	/// the font to be used on screen
	MathTextCodes code_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
