// -*- C++ -*-
#ifndef MATH_BINARYOPINSET_H
#define MATH_BINARYOPINSET_H

#include "math_nestinset.h"
#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for multiplication
    \author André Pönitz
 */
class MathBinaryOpInset : public MathNestInset {
public:
	///
	explicit MathBinaryOpInset(char op);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(NormalStream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
private:
	///
	int opwidth() const;
	///
	char op_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
