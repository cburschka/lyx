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
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
};

#endif
