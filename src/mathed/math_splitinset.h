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
	void write(WriteStream & os) const;
	///
	int defaultColSpace(col_type) { return 0; }
	///
	char defaultColAlign(col_type) { return 'l'; }
};

#endif
