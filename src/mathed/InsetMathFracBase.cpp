/**
 * \file InsetMathFracBase.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathFracBase.h"
#include "MathData.h"
#include "Cursor.h"


namespace lyx {


InsetMathFracBase::InsetMathFracBase(idx_type ncells)
	: InsetMathNest(ncells)
{}


bool InsetMathFracBase::idxRight(Cursor &) const
{
	return false;
}


bool InsetMathFracBase::idxLeft(Cursor &) const
{
	return false;
}


bool InsetMathFracBase::idxUpDown(Cursor & cur, bool up) const
{
	InsetMath::idx_type target = !up; // up ? 0 : 1, since upper cell has idx 0
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(cur.x_target());
	return true;
}


} // namespace lyx
