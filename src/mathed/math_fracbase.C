#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fracbase.h"
#include "math_mathmlstream.h"


MathFracbaseInset::MathFracbaseInset()
	: MathNestInset(2)
{}


bool MathFracbaseInset::idxRight(idx_type &, pos_type &) const
{
	return false;
}


bool MathFracbaseInset::idxLeft(idx_type &, pos_type &) const
{
	return false;
}


bool MathFracbaseInset::idxUp(idx_type & idx) const
{
	if (idx == 0)
		return false;
	idx = 0;
	return true;
}


bool MathFracbaseInset::idxDown(idx_type & idx) const
{
	if (idx == 1)
		return false;
	idx = 1;
	return true;
}
