// -*- C++ -*-
#ifndef MATH_NUMBERINSET_H
#define MATH_NUMBERINSET_H

#include "math_inset.h"


/** Some inset that "is" a number
 *  maily for math-extern
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

class MathNumberInset : public MathInset {
public:
	///
	explicit MathNumberInset(string const & s);
	///
	MathInset * clone() const;
	///
	Dimension metrics(MetricsInfo & mi) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	string str() const { return str_; }
	///
	MathNumberInset * asNumberInset() { return this; }

	///
	void normalize(NormalStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;

private:
	/// the number as string
	string str_;
};
#endif
