#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H

#include "math_hullinset.h"

class MathParInset : public MathHullInset {
public:
	///
	MathParInset();
	///
	MathParInset * asParInset() { return this; }
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
};

#endif
