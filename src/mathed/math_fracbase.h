// -*- C++ -*-
#ifndef MATH_FRACBASE_H
#define MATH_FRACBASE_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathFracbaseInset : public MathNestInset {
public:
	///
	MathFracbaseInset();
	///
	bool idxUp(MathInset::idx_type &, MathInset::pos_type &) const;
	///
	bool idxDown(MathInset::idx_type &, MathInset::pos_type &) const;
	///
	bool idxLeft(MathInset::idx_type &, MathInset::pos_type &) const;
	///
	bool idxRight(MathInset::idx_type &, MathInset::pos_type &) const;
};

#endif
