// -*- C++ -*-
#ifndef MATH_BRACEINSET_H
#define MATH_BRACEINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Extra nesting
    \author André Pönitz
*/

class MathBraceInset : public MathNestInset {
public:
	///
	MathBraceInset();
	///
	MathInset * clone() const;
	/// we write extra braces in any case...
	bool extraBraces() const { return true; }
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void infoize(std::ostream & os) const;

private:
	/// width of brace character
	mutable int wid_;
};

#endif
