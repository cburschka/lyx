// -*- C++ -*-
#ifndef MATH_BINOMINSET_H
#define MATH_DINOMINSET_H

#include "math_fracbase.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Binom like objects
    \author André Pönitz 
 */
class MathBinomInset : public MathFracbaseInset {
public:
	///
	MathBinomInset();
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
private:
	///
	int dw() const;
};

#endif
