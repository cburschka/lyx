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
#include "Painter.h"


MathRootInset::MathRootInset()
	: MathNestInset(2)
{}


MathInset * MathRootInset::clone() const
{
	return new MathRootInset(*this);
}


void MathRootInset::metrics(MathMetricsInfo const & mi) const
{
	MathNestInset::metrics(mi);
	ascent_  = std::max(xcell(0).ascent()  + 5, xcell(1).ascent())  + 2;
	descent_ = std::max(xcell(1).descent() + 5, xcell(0).descent()) + 2;
	width_   = xcell(0).width() + xcell(1).width() + 10;
}


void MathRootInset::draw(Painter & pain, int x, int y) const
{
	int const w = xcell(0).width();
	xcell(0).draw(pain, x, y - 5 - xcell(0).descent());       // the "exponent"
	xcell(1).draw(pain, x + w + 8, y);   // the "base"
	int const a = ascent();
	int const d = descent();
	int xp[5];
	int yp[5];
	xp[0] = x + width_;   yp[0] = y - a + 1;
	xp[1] = x + w + 4;    yp[1] = y - a + 1;
	xp[2] = x + w;        yp[2] = y + d;
	xp[3] = x + w - 2;    yp[3] = y + (d - a)/2 + 2;
	xp[4] = x;            yp[4] = y + (d - a)/2 + 2;
	pain.lines(xp, yp, 5, LColor::math);
}


void MathRootInset::write(WriteStream & os) const
{
	os << "\\sqrt[" << cell(0) << "]{" << cell(1) << '}';
}


void MathRootInset::normalize(NormalStream & os) const
{
	os << "[root " << cell(1) << ' ' << cell(1) << ']';
}


bool MathRootInset::idxUp(idx_type & idx) const
{
	if (idx == 0)
		return false;
	idx = 0;
	return true;
}


bool MathRootInset::idxDown(idx_type & idx) const
{
	if (idx == 1)
		return false;
	idx = 1;
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
