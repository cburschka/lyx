// -*- C++ -*-
#ifndef MATH_EXINTINSET_H
#define MATH_EXINTINSET_H

// /\int_l^u f(x) dxin one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

#include "math_scriptinset.h"

class MathExIntInset : public MathInset {
public:
	///
	MathExIntInset();
	///
	MathInset * clone() const;
	///
	void differential(MathArray const &);
	///
	void core(MathArray const &);
	///
	void scripts(MathAtom const &);
	///
	void symbol(MathAtom const &);
	///
	bool hasScripts() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;
private:
	///
	MathAtom int_;
	///
	MathAtom scripts_;
	///
	MathArray core_;
	///
	MathArray diff_;
};
#endif
