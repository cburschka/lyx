// -*- C++ -*-
#ifndef MATH_SUBSTACK_H
#define MATH_SUBSTACK_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathSubstackInset : public MathGridInset {
public:
	///
	MathSubstackInset();
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	MathSubstackInset const * asSubstackInset() const { return this; }

	///
	void normalize();
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
	///
	void maplize(MapleStream &) const;
};

#endif
