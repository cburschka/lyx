// -*- C++ -*-
#ifndef MATH_DECORATIONINSET_H
#define MATH_DECORATIONINSET_H

#include "math_inset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Decorations over (below) a math object
    \author Alejandro Aguilar Sierra
 */
class MathDecorationInset : public MathInset {
public:
	///
	explicit MathDecorationInset(int);
	///
	MathInset * clone() const;
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile) const;
	///
	void Metrics(MathStyles st);
private:
	///
	int deco_;
	///
	bool upper_;
	/// height of deco
	int dh_;
	/// vertical offset of deco
	int dy_;
};
#endif
