/**
 * \file math_fracbase.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_fracbase.h"
#include "math_data.h"


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


bool MathFracbaseInset::idxUpDown(idx_type & idx, pos_type & pos, bool up,
	int targetx) const
{
	MathInset::idx_type target = !up; // up ? 0 : 1, since upper cell has idx 0
	if (idx == target)
		return false;
	idx = target;
	pos = cell(idx).x2pos(targetx);
	return true;
}
