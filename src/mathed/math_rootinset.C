/*
 *  File:        math_root.C
 *  Purpose:     Implementation of the root object
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     January 1999
 *  Description: Root math object
 *
 *  Copyright: 1999 Alejandro Aguilar Sierra
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_rootinset.h"
#include "math_mathmlstream.h"
#include "frontends/Painter.h"


using std::max;


MathRootInset::MathRootInset()
	: MathNestInset(2)
{}


MathInset * MathRootInset::clone() const
{
	return new MathRootInset(*this);
}


void MathRootInset::metrics(MathMetricsInfo & mi) const
{
	MathNestInset::metrics(mi);
	dim_.a = max(cell(0).ascent()  + 5, cell(1).ascent())  + 2;
	dim_.d = max(cell(1).descent() + 5, cell(0).descent()) + 2;
	dim_.w = cell(0).width() + cell(1).width() + 10;
	metricsMarkers();
}


void MathRootInset::draw(MathPainterInfo & pi, int x, int y) const
{
	int const w = cell(0).width();
	// the "exponent"
	cell(0).draw(pi, x, y - 5 - cell(0).descent());
	// the "base"
	cell(1).draw(pi, x + w + 8, y);
	int const a = ascent();
	int const d = descent();
	int xp[5];
	int yp[5];
	xp[0] = x + width();  yp[0] = y - a + 1;
	xp[1] = x + w + 4;    yp[1] = y - a + 1;
	xp[2] = x + w;        yp[2] = y + d;
	xp[3] = x + w - 2;    yp[3] = y + (d - a)/2 + 2;
	xp[4] = x;            yp[4] = y + (d - a)/2 + 2;
	pi.pain.lines(xp, yp, 5, LColor::math);
	drawMarkers(pi, x, y);
}


void MathRootInset::write(WriteStream & os) const
{
	os << "\\sqrt[" << cell(0) << "]{" << cell(1) << '}';
}


void MathRootInset::normalize(NormalStream & os) const
{
	os << "[root " << cell(0) << ' ' << cell(1) << ']';
}


bool MathRootInset::idxUpDown(idx_type & idx, pos_type & pos, bool up, int) const
{
	bool target = !up; // up ? 0 : 1;
	if (idx == target)
		return false;
	idx = target;
	pos = target ? 0 : cell(0).size();
	return true;
}


void MathRootInset::octavize(OctaveStream & os) const
{
	os << "root(" << cell(1) << ',' << cell(0) <<')';
}


void MathRootInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mroot") << cell(1) << cell(0) << ETag("mroot");
}
