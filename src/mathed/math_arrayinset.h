// -*- C++ -*-
#ifndef MATH_ARRAYINSET_H
#define MATH_ARRAYINSET_H

#include "math_grid.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathArrayInset : public MathGridInset {
public: 
	///
	MathArrayInset(int m, int n);
	///
	MathInset *  Clone() const;
	///
	void Write(std::ostream &, bool fragile) const;
};

#endif
