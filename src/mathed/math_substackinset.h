// -*- C++ -*-
#ifndef MATH_SUBSTACK_H
#define MATH_SUBSTACK_H

#include "math_gridinset.h"


/// support for AMS's \\substack

class MathSubstackInset : public MathGridInset {
public:
	///
	MathSubstackInset();
	///
	MathInset * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	MathSubstackInset const * asSubstackInset() const { return this; }

	///
	void normalize();
	///
	void infoize(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maple(MapleStream &) const;
};

#endif
