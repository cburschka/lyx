// -*- C++ -*-
#ifndef MATH_XYARROWINSET_H
#define MATH_ARROWINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathXYArrowInset : public MathNestInset {
public: 
	///
	MathXYArrowInset();
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo const & st) const;
	///
	MathXYArrowInset * asXYArrowInset() { return this; }
	///
	void normalize();

	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream &) const;
};

#endif
