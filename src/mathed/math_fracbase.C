#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fracbase.h"


MathFracbaseInset::MathFracbaseInset()
	: MathNestInset(2)
{}


bool MathFracbaseInset::idxRight(MathInset::idx_type &,
				 MathInset::pos_type &) const
{
	return false;
}


bool MathFracbaseInset::idxLeft(MathInset::idx_type &,
				MathInset::pos_type &) const
{
	return false;
}


bool MathFracbaseInset::idxUp(MathInset::idx_type & idx,
			      MathInset::pos_type & pos) const
{
	if (idx == 0)
		return false;
	idx = 0;
	pos = std::min(pos, cell(idx).size());
	return true;
}


bool MathFracbaseInset::idxDown(MathInset::idx_type & idx,
				MathInset::pos_type & pos) const
{
	if (idx == 1)
		return false;
	idx = 1;
	pos = std::min(pos, cell(idx).size());
	return true;
}
