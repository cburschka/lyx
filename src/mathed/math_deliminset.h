// -*- C++ -*-
#ifndef MATH_DELIMINSET_H
#define MATH_DELIMINSET_H

#include "math_parinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** A delimiter
    \author Alejandro Aguilar Sierra
*/
class MathDelimInset : public MathParInset {
public:
	///
	MathDelimInset(int, int, short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int, int);
	///
	void Write(std::ostream &, bool fragile);
	///
	void Metrics();
private:
	///
	int left_;
	///
	int right_;
	///
	int dw_;
	///
	int dh_;
};
#endif
