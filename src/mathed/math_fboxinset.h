// -*- C++ -*-
#ifndef MATH_FBOXINSET_H
#define MATH_FBOXINSET_H

#include "math_nestinset.h"
#include "math_metricsinfo.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Extra nesting
    \author André Pönitz
*/

class MathFboxInset : public MathNestInset {
public:
	///
	MathFboxInset();
	///
	MathInset * clone() const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
};

#endif
