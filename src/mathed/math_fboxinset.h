// -*- C++ -*-
#ifndef MATH_FBOXINSET_H
#define MATH_FBOXINSET_H

#include "math_nestinset.h"
#include "metricsinfo.h"


/** Extra nesting
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
*/

class latexkeys;

class MathFboxInset : public MathNestInset {
public:
	///
	MathFboxInset(latexkeys const * key);
	///
	MathInset * clone() const;
	///
	mode_type currentMode() const;
	///
	Dimension metrics(MetricsInfo & mi) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
private:
	///
	latexkeys const * key_;
};

#endif
