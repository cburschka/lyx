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
	bool idxUp(int &, int &) const;
	///
	bool idxDown(int &, int &) const;
	///
	bool idxLeft(int &, int &) const;
	///
	bool idxRight(int &, int &) const;
	///
	bool idxFirstUp(int & idx, int & pos) const;
	///
	bool idxFirstDown(int & idx, int & pos) const;
	///
	bool idxLastUp(int & idx, int & pos) const;
	///
	bool idxLastDown(int & idx, int & pos) const;
};

#endif
