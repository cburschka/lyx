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


bool MathFracbaseInset::idxUpDown(idx_type & idx, bool up) const
{
	int target = !up;   // up ? 0 : 1, since upper cell has idx 0
	if (idx == target)
		return false;
	idx = target;
	return true;
}
