// -*- C++ -*-
#ifndef MATH_EXFUNCINSET_H
#define MATH_EXFUNCINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "math_nestinset.h"

// f(x) in one block (as opposed to 'f','(','x',')' or 'f','x')
// for interfacing external programs

class MathExFuncInset : public MathNestInset {
public:
	///
	explicit MathExFuncInset(string const & name);
	///
	MathExFuncInset(string const & name, MathArray const & ar);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	///
	string name() const;

	///
	void maplize(MapleStream &) const;
	///
	void mathematicize(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octavize(OctaveStream &) const;

private:
	///
	string const name_;
};
#endif
