// -*- C++ -*-
#ifndef MATH_SPACEINSET_H
#define MATH_SPACEINSET_H

#include "math_diminset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/// Smart spaces
class MathSpaceInset : public MathDimInset {
public:
	///
	explicit MathSpaceInset(int sp);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
	///
	MathSpaceInset const * asSpaceInset() const { return this; }
	///
	MathSpaceInset * asSpaceInset() { return this; }
	///
	void incSpace();
private:
	///
	int space_;
};
#endif
