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
#include "cursor.h"


MathFracbaseInset::MathFracbaseInset()
	: MathNestInset(2)
{}


bool MathFracbaseInset::idxRight(LCursor &) const
{
	return false;
}


bool MathFracbaseInset::idxLeft(LCursor &) const
{
	return false;
}


bool MathFracbaseInset::idxUpDown(LCursor & cur, bool up) const
{
	MathInset::idx_type target = !up; // up ? 0 : 1, since upper cell has idx 0
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(cur.x_target());
	return true;
}
