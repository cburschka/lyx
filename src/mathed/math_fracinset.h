// -*- C++ -*-
#ifndef MATH_FRACINSET_H
#define MATH_FRACINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Fraction like objects (frac, stackrel, binom)
    \author Alejandro Aguilar Sierra
 */
class MathFracInset : public MathNestInset {
public:
	///
	explicit MathFracInset(const string & name);
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
	///
	bool idxUp(int &, int &) const;
	///
	bool idxDown(int &, int &) const;
	///
	bool idxLeft(int &, int &) const;
	///
	bool idxRight(int &, int &) const;
	///
	bool idxFirstUp(int & idx, int & pos) const;
	///
	bool idxFirstDown(int & idx, int & pos) const;
	///
	bool idxLastUp(int & idx, int & pos) const;
	///
	bool idxLastDown(int & idx, int & pos) const;
};

#endif
