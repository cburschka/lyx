// -*- C++ -*-
#ifndef MATH_SPLITINSET_H
#define MATH_SPLITINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathSplitInset : public MathGridInset {
public: 
	///
	explicit MathSplitInset(int n);
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	int defaultColSpace(int) { return 0; }
	///
	char defaultColAlign(int) { return 'l'; }
};

#endif
