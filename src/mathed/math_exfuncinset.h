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
	void write(MathWriteInfo & os) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	string octavize() const;

private:
	///
	string const name_;
	///
	mutable MathMetricsInfo mi_;
};
#endif
