#ifndef MATH_DOTSINSET_H
#define MATH_DOTSINSET_H

#include "math_inset.h"

///
class MathDotsInset: public MathedInset {
public:
	///
	MathDotsInset(string const &, int, short st = LM_ST_TEXT);
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
	int dh, code;
};   
#endif
