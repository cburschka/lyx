// -*- C++ -*-
#ifndef MATH_DELIMINSET_H
#define MATH_DELIMINSET_H

#include "math_nestinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A delimiter
    \author Alejandro Aguilar Sierra
*/

class latexkeys;

class MathDelimInset : public MathNestInset {
public:
	///
	MathDelimInset(latexkeys const *, latexkeys const *);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st) const;
private:
	///
	static string latexName(latexkeys const *);
	///
	int dw() const;
	///
	latexkeys const * left_;
	///
	latexkeys const * right_;
};
#endif
