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
	bool idxUpDown(idx_type &, bool up) const;
	///
	bool idxLeft(idx_type &, pos_type &) const;
	///
	bool idxRight(idx_type &, pos_type &) const;
};

#endif
