// -*- C++ -*-
#ifndef MATH_FUNCINSET_H
#define MATH_FUNCINSET_H

#include "math_diminset.h"
#include "math_defs.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
 Functions or LaTeX names for objects that I don't know how to draw.
 */
class MathFuncInset : public MathDimInset {
public:
	///
	explicit MathFuncInset(string const & nm);
	///
	MathInset * clone() const;
	///
	void MathFuncInset::metrics(MathStyles st);
	///
	void draw(Painter &, int, int);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
};
#endif
