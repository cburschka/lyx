
#ifndef MATH_SQRTINSET_H
#define MATH_SQRTINSET_H

#include "math_parinset.h"

/** The square root inset.
    \author Alejandro Aguilar Siearra
 */
class MathSqrtInset : public MathParInset {
public:
	///
	MathSqrtInset(short st = LM_ST_TEXT);
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile);
	///
	void Metrics();
	///
	bool Inside(int, int);
private:
	///
	int hmax_;
	///
	int wbody_;
};
#endif
