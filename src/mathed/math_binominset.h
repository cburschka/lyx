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
	explicit MathBinomInset(bool choose = false);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
private:
	///
	int dw() const;
	///
	bool choose_;
};

#endif
