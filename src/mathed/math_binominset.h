// -*- C++ -*-
#ifndef MATH_BINOMINSET_H
#define MATH_DINOMINSET_H

#include "math_fracbase.h"


/** Binom like objects
 *  \author André Pönitz
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
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
private:
	///
	int dw() const;
	///
	bool choose_;
};

#endif
