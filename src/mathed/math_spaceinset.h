// -*- C++ -*-
#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "math_inset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/// Smart spaces
class MathSpaceInset : public MathInset {
public:
	///
	explicit MathSpaceInset(int sp);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int, int);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st);
	///
	bool isSpaceInset() const { return true; }
	///
	void incSpace();
private:
	///
	int space_;
};
#endif
