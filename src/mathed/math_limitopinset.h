// -*- C++ -*-
#ifndef MATH_LIMITOPINSET_H
#define MATH_LIMITOPINSET_H

// /\sum_l^u f(x) in one block
// for interfacing external programs

#include "math_scriptinset.h"

class MathLimitOpInset : public MathInset {
public:
	///
	MathLimitOpInset(MathScriptInset const &, MathArray const &);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
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
