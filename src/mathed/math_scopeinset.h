// -*- C++ -*-
#ifndef MATH_SCOPEINSET_H
#define MATH_SCOPEINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for new scopes (i.e.  {....} blocks in LaTeX)
    \author André Pönitz
 */
class MathScopeInset : public MathNestInset {
public:
	///
	MathScopeInset();
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
};
#endif
