// -*- C++ -*-
#ifndef MATH_SPLITINSET_H
#define MATH_SPLITINSET_H

#include "math_gridinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathSplitInset : public MathGridInset {
public:
	///
	explicit MathSplitInset(string const & name);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	int defaultColSpace(col_type) { return 0; }
	///
	char defaultColAlign(col_type);
private:
	///
	string name_;
};

#endif
