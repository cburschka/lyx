// -*- C++ -*-
#ifndef MATH_DECORATIONINSET_H
#define MATH_DECORATIONINSET_H

#include "math_parinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** Decorations over (below) a math object
    \author Alejandro Aguilar Sierra
 */
class MathDecorationInset : public MathParInset {
public:
	///
	MathDecorationInset(int, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile);
	///
	void Metrics();
	///
	bool GetLimits() const;
private:
	///
	int deco_;
	///
	bool upper_;
	///
	int dw_;
	///
	int dh_;
	///
	int dy_;
};
#endif
