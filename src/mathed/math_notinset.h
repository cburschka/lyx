// -*- C++ -*-
#ifndef MATH_NOTINSET_H
#define MATH_NOTINSET_H

#include "math_diminset.h"

struct latexkeys;

// \\not

class MathNotInset : public MathDimInset {
public:
	///
	MathNotInset();
	///
	MathInset * clone() const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
};
#endif
