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
	bool idxUp(idx_type &) const;
	///
	bool idxDown(idx_type &) const;
	///
	bool idxLeft(idx_type &, pos_type &) const;
	///
	bool idxRight(idx_type &, pos_type &) const;
	///
	bool idxFirstUp(idx_type &, pos_type &) const;
	///
	bool idxFirstDown(idx_type &, pos_type &) const;
	///
	bool idxLastUp(idx_type &, pos_type &) const;
	///
	bool idxLastDown(idx_type &, pos_type &) const;
};

#endif
