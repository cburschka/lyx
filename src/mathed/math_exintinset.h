// -*- C++ -*-
#ifndef MATH_EXINTINSET_H
#define MATH_EXINTINSET_H

// \int_l^u f(x) dx in one block (as opposed to 'f','(','x',')' or 'f','x')
// or \sum, \prod...  for interfacing external programs

#include "math_nestinset.h"

// cell(0) is stuff before the 'd', cell(1) the stuff after
class MathExIntInset : public MathNestInset {
public:
	///
	explicit MathExIntInset(string const & name_);
	///
	MathInset * clone() const;
	///
	void symbol(string const &);
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
	///
	void mathematicize(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;
private:
	///
	bool hasScripts() const;

	///
	string symbol_;
};

#endif
