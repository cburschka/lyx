#ifndef MATH_PARINSET_H
#define MATH_PARINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_hullinset.h"

class MathParInset : public MathHullInset {
public:
	///
	MathParInset() {}
	///
	MathParInset(MathArray const & ar);
	///
	mode_type currentMode() const { return TEXT_MODE; }
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
