// -*- C++ -*-
#ifndef MATH_UNDERSETINSET_H
#define MATH_UNDERSETINSET_H

#include "math_fracbase.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Underset objects
    \author André Pönitz
 */
class MathUndersetInset : public MathFracbaseInset {
public:
	///
	MathUndersetInset();
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
};

#endif
