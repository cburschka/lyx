// -*- C++ -*-
#ifndef MATH_DECORATIONINSET_H
#define MATH_DECORATIONINSET_H

#include "math_nestinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Decorations and accents over (below) a math object
    \author Alejandro Aguilar Sierra
 */

struct latexkeys;

class MathDecorationInset : public MathNestInset {
public:
	///
	explicit MathDecorationInset(latexkeys const *);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int x, int y) const;
	///
	void write(std::ostream &, bool fragile) const;
	///
	void metrics(MathStyles st) const;
	///
	void writeNormal(std::ostream & os) const;
private:
	///
	latexkeys const * key_;
	///
	bool upper_;
	/// height cache of deco
	mutable int dh_;
	/// vertical offset cache of deco
	mutable int dy_;
};
#endif
