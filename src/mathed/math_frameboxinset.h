// -*- C++ -*-
#ifndef MATH_FRAMEBOXINSET_H
#define MATH_FRAMEBOXINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Extra nesting
    \author André Pönitz
*/

class MathFrameboxInset : public MathNestInset {
public:
	///
	MathFrameboxInset();
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
private:
	/// width of '[' in current font
	mutable int w_;
};

#endif
