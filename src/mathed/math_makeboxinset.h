// -*- C++ -*-
#ifndef MATH_MAKEBOXINSET_H
#define MATH_MAKEBOXINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Extra nesting: \\makebox.
 *  \author Ling Li
 *
 *  Full author contact details are available in file CREDITS
 */

// consolidate with MathFrameboxInset?

class MathMakeboxInset : public MathNestInset {
public:
	///
	MathMakeboxInset();
	///
	InsetBase * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	/// write normalized content
	void normalize(NormalStream & ns) const;
	///
	mode_type currentMode() const { return TEXT_MODE; }
private:
	/// width of '[' in current font
	mutable int w_;
};

#endif
