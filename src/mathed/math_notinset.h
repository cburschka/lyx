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
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;
	///
	void metrics(MathMetricsInfo const & mi) const;
	///
	void draw(Painter &, int x, int y) const;
private:
	///
	mutable LyXFont font_;
};
#endif
