// -*- C++ -*-
#ifndef MATH_STRINGINSET_H
#define MATH_STRINGINSET_H

#include "math_inset.h"


/** Some collection of chars with similar properties
 *  maily for math-extern
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

class MathStringInset : public MathInset {
public:
	///
	explicit MathStringInset(string const & s);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	string str() const { return str_; }
	///
	MathStringInset * asStringInset() { return this; }

	///
	void normalize(NormalStream &) const;
	///
	void octave(OctaveStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;

private:
	/// the string
	string str_;
};
#endif
