// -*- C++ -*-
#ifndef MATH_FUNCINSET_H
#define MATH_FUNCINSET_H

#include "math_diminset.h"
#include "math_defs.h"
#include "LString.h"

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
	void metrics(MathStyles st) const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void writeNormal(std::ostream &) const;
	///
	string const & name() const;
	///
	void setName(string const & n);
private:
	///
	string name_;
};
#endif
