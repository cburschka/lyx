// -*- C++ -*-
#ifndef MATH_EXINTINSET_H
#define MATH_EXINTINSET_H

// /\int_l^u f(x) dxin one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

#include "math_scriptinset.h"

class MathExIntInset : public MathInset {
public:
	///
	MathExIntInset(MathScriptInset const &, MathArray const &, MathArray const &);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
private:
	///
	MathAtom int_;
	///
	MathScriptInset scripts_;
	///
	MathArray core_;
	///
	MathArray diff_;
};
#endif
