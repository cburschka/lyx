#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fracbase.h"


MathFracbaseInset::MathFracbaseInset()
	: MathNestInset(2)
{}


bool MathFracbaseInset::idxRight(int &, int &) const
{
	return false;
}


bool MathFracbaseInset::idxLeft(int &, int &) const
{
	return false;
}


bool MathFracbaseInset::idxUp(int & idx, int &) const
{
	if (idx == 0)
		return false;
	idx = 0;
	return true;
}


bool MathFracbaseInset::idxDown(int & idx, int &) const
{
	if (idx == 1)
		return false;
	idx = 1;
	return true;
}
