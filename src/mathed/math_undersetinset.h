// -*- C++ -*-

/** Underset objects
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS
 */

#ifndef MATH_UNDERSETINSET_H
#define MATH_UNDERSETINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_fracbase.h"

/// Inset for underset
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
