// -*- C++ -*-
#ifndef MATH_DIFFINSET_H
#define MATH_DIFFINSET_H

#ifdef __GNUG__
#pragma interface
#endif

// d f(x)/dx in one block
// for interfacing external programs

#include "math_nestinset.h"

class MathDiffInset : public MathNestInset {
public:
	///
	explicit MathDiffInset();
	///
	MathInset * clone() const;
	///
	void addDer(MathArray const & der);
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;

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
};

#endif
