// -*- C++ -*-
#ifndef MATH_BRACEINSET_H
#define MATH_BRACEINSET_H

#include "math_nestinset.h"
#include "math_metricsinfo.h"

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
	///
	MathBraceInset * asBraceInset() { return this; }
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream &) const;
	///
	void metrics(MathMetricsInfo & st) const;

private:
	/// width of brace character
	mutable int wid_;
};

#endif
