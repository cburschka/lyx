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
	void Write(std::ostream &, bool fragile) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Metrics(MathStyles st, int asc = 0, int des = 0);
	///
	void SetSpace(int sp);
	///
	int GetSpace();
private:
	///
	int space_;
};
#endif
