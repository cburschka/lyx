/**
 * \file math_undersetinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_undersetinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"


using std::max;
using std::auto_ptr;


auto_ptr<InsetBase> MathUndersetInset::clone() const
{
	return auto_ptr<InsetBase>(new MathUndersetInset(*this));
}


void MathUndersetInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(1).metrics(mi);
	FracChanger dummy(mi.base);
	cell(0).metrics(mi);
	dim_.wid = max(cell(0).width(), cell(1).width()) + 4;
	dim_.asc = cell(1).ascent();
	dim_.des = cell(1).descent() + cell(0).height() + 4;
	dim = dim_;
}


void MathUndersetInset::draw(PainterInfo & pi, int x, int y) const
{
	int m  = x + pi.width / 2;
	int yo = y + cell(1).descent() + cell(0).ascent() + 1;
	cell(1).draw(pi, m - cell(1).width() / 2, y);
	FracChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, yo);
}


bool MathUndersetInset::idxFirst(BufferView & bv) const
{
	CursorSlice & cur = cursorTip(bv);
	cur.idx() = 1;
	cur.pos() = 0;
	return true;
}


bool MathUndersetInset::idxLast(BufferView & bv) const
{
	CursorSlice & cur = cursorTip(bv);
	cur.idx() = 1;
	cur.pos() = cur.lastpos();
	return true;
}


bool MathUndersetInset::idxUpDown(BufferView & bv, bool up, int targetx) const
{
	CursorSlice & cur = cursorTip(bv);
	idx_type target = up; // up ? 1 : 0, since upper cell has idx 1
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cur.cell().x2pos(targetx);
	return true;
}


void MathUndersetInset::write(WriteStream & os) const
{
	os << "\\underset{" << cell(0) << "}{" << cell(1) << '}';
}


void MathUndersetInset::normalize(NormalStream & os) const
{
	os << "[underset " << cell(0) << ' ' << cell(1) << ']';
}
