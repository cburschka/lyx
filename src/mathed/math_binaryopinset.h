// -*- C++ -*-
#ifndef MATH_BINARYOPINSET_H
#define MATH_BINARYOPINSET_H

#include "math_nestinset.h"
#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** An inset for multiplication
 *  \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */
class MathBinaryOpInset : public MathNestInset {
public:
	///
	explicit MathBinaryOpInset(char op);
	///
	MathInset * clone() const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MathMetricsInfo & st) const;
private:
	///
	int opwidth() const;
	///
	char op_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
