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
	MathExFuncInset(string const & name, MathArray const & ar);
	///
	MathInset * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	string name() const;

	///
	void maple(MapleStream &) const;
	///
	void maxima(MaximaStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void octave(OctaveStream &) const;

private:
	///
	string const name_;
};
#endif
