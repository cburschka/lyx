/**
 * \file math_rootinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_rootinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "cursor.h"
#include "LColor.h"

#include "frontends/Painter.h"

using std::max;
using std::auto_ptr;



MathRootInset::MathRootInset()
	: MathNestInset(2)
{}


auto_ptr<InsetBase> MathRootInset::clone() const
{
	return auto_ptr<InsetBase>(new MathRootInset(*this));
}


void MathRootInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	MathNestInset::metrics(mi);
	dim_.asc = max(cell(0).ascent()  + 5, cell(1).ascent())  + 2;
	dim_.des = max(cell(1).descent() + 5, cell(0).descent()) + 2;
	dim_.wid = cell(0).width() + cell(1).width() + 10;
	metricsMarkers(1);
	dim = dim_;
}


void MathRootInset::draw(PainterInfo & pi, int x, int y) const
{
	int const w = cell(0).width();
	// the "exponent"
	cell(0).draw(pi, x, y - 5 - cell(0).descent());
	// the "base"
	cell(1).draw(pi, x + w + 8, y);
	int const a = dim_.ascent();
	int const d = dim_.descent();
	int xp[5];
	int yp[5];
	xp[0] = x + dim_.width();  yp[0] = y - a + 1;
	xp[1] = x + w + 4;         yp[1] = y - a + 1;
	xp[2] = x + w;             yp[2] = y + d;
	xp[3] = x + w - 2;         yp[3] = y + (d - a)/2 + 2;
	//xp[4] = x;                 yp[4] = y + (d - a)/2 + 2;
	xp[4] = x + w - 5;         yp[4] = y + (d - a)/2 + 4;
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


bool MathRootInset::idxUpDown(LCursor & cur, bool up) const
{
	bool target = !up; // up ? 0 : 1;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = up ? cur.lastpos() : 0;
	return true;
}


void MathRootInset::maple(MapleStream & os) const
{
	os << '(' << cell(1) << ")^(1/(" << cell(0) <<"))";
}


void MathRootInset::octave(OctaveStream & os) const
{
	os << "root(" << cell(1) << ',' << cell(0) << ')';
}


void MathRootInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mroot") << cell(1) << cell(0) << ETag("mroot");
}
