// -*- C++ -*-
#ifndef MATH_EXINTINSET_H
#define MATH_EXINTINSET_H

// /\int_l^u f(x) dxin one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

#include "math_scriptinset.h"

class MathExIntInset : public MathInset {
public:
	///
	explicit MathExIntInset(string const & name_);
	///
	MathInset * clone() const;
	///
	void index(MathArray const &);
	///
	void core(MathArray const &);
	///
	void scripts(MathAtom const &);
	///
	MathAtom & scripts();
	///
	void symbol(string const &);
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
	bool hasScripts() const;

	///
	string symbol_;
	///
	MathAtom scripts_;
	///
	MathArray core_;
	///
	MathArray index_;
};

#endif
