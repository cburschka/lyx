// -*- C++ -*-
#ifndef MATH_DIFFINSET_H
#define MATH_DIFFINSET_H


// d f(x)/dx in one block
// for interfacing external programs

#include "math_nestinset.h"

class MathDiffInset : public MathNestInset {
public:
	///
	explicit MathDiffInset();
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void addDer(MathArray const & der);
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;

	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
	///
	void mathematica(MathematicaStream &) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void write(WriteStream & os) const;
};

#endif
