// -*- C++ -*-
#ifndef MATH_SQRTINSET_H
#define MATH_SQRTINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** The square root inset.
    \author Alejandro Aguilar Siearra
 */
class MathSqrtInset : public MathInset {
public:
	///
	MathSqrtInset();
	///
	MathInset * Clone() const;
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st);
};
#endif
