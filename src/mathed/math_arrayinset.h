// -*- C++ -*-
#ifndef MATH_ARRAYINSET_H
#define MATH_ARRAYINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathArrayInset : public MathGridInset {
public: 
	///
	MathArrayInset(int m, int n);
	///
	virtual MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	bool isArray() const { return true; }
};

#endif
