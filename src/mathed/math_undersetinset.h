// -*- C++ -*-

/** Underset objects
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS
 */

#ifndef MATH_UNDERSETINSET_H
#define MATH_UNDERSETINSET_H


#include "math_fracbase.h"

/// Inset for underset
class MathUndersetInset : public MathFracbaseInset {
public:
	///
	MathUndersetInset();
	///
	InsetBase * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
};

#endif
