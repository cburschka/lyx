// -*- C++ -*-
#ifndef MATH_NOTINSET_H
#define MATH_NOTINSET_H

#include "math_diminset.h"

// \\not

class MathNotInset : public MathDimInset {
public:
	///
	MathNotInset();
	///
	MathInset * clone() const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(NormalStream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
private:
	///
	mutable MathMetricsInfo mi_;
};
#endif
