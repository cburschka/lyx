// -*- C++ -*-
#ifndef MATH_DECORATIONINSET_H
#define MATH_DECORATIONINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Decorations and accents over (below) a math object
    \author Alejandro Aguilar Sierra
 */

struct latexkeys;

class MathDecorationInset : public MathInset {
public:
	///
	explicit MathDecorationInset(latexkeys const *);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int, int);
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st);
	///
	void writeNormal(std::ostream & os) const;
private:
	///
	latexkeys const * key_;
	///
	bool upper_;
	/// height of deco
	int dh_;
	/// vertical offset of deco
	int dy_;
};
#endif
