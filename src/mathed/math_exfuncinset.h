// -*- C++ -*-
#ifndef MATH_EXFUNCINSET_H
#define MATH_EXFUNCINSET_H

#include "math_nestinset.h"

// f(x) in one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

class MathExFuncInset : public MathNestInset {
public:
	///
	explicit MathExFuncInset(string const & name);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;
	///
	void write(WriteStream & os) const;

private:
	///
	string const name_;
};
#endif
