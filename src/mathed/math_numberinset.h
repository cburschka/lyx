// -*- C++ -*-
#ifndef MATH_NUMBERINSET_H
#define MATH_NUMBERINSET_H

#include "math_diminset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Some inset that "is" a number
 *  maily for math-extern
 *  \author André Pönitz
 */

class MathNumberInset : public MathDimInset {
public:
	///
	explicit MathNumberInset(string const & s);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	string str() const { return str_; }
	///
	MathNumberInset * asNumberInset() { return this; }

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

private:
	/// the number as string
	string str_;
};
#endif
