// -*- C++ -*-
#ifndef MATH_DELIMINSET_H
#define MATH_DELIMINSET_H

#include "math_parinset.h"

/// A delimiter
class MathDelimInset: public MathParInset {
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
protected:
	///
	int left;
	///
	int right;
	///
	int dw;
	///
	int dh;
};
#endif
