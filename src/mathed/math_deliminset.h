// -*- C++ -*-
#ifndef MATH_DELIMINSET_H
#define MATH_DELIMINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A delimiter
    \author Alejandro Aguilar Sierra
*/
class MathDelimInset : public MathInset {
public:
	///
	MathDelimInset(int, int);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void Metrics(MathStyles st, int asc = 0, int des = 0);
private:
	int dw() const;
	///
	int left_;
	///
	int right_;
};
#endif
