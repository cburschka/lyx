// -*- C++ -*-
#ifndef MATH_STACKRELINSET_H
#define MATH_STACKRELINSET_H

#include "math_fracbase.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Stackrel objects 
    \author André Pönitz
 */
class MathStackrelInset : public MathFracbaseInset {
public:
	///
	MathStackrelInset();
	///
	MathInset * clone() const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
};

#endif
