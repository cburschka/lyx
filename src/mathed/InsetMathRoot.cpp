/**
 * \file InsetMathRoot.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathRoot.h"
#include "MathData.h"
#include "MathStream.h"
#include "Cursor.h"
#include "Color.h"

#include "frontends/Painter.h"


namespace lyx {


InsetMathRoot::InsetMathRoot()
	: InsetMathNest(2)
{}


Inset * InsetMathRoot::clone() const
{
	return new InsetMathRoot(*this);
}


void InsetMathRoot::metrics(MetricsInfo & mi, Dimension & dim) const
{
	using std::max;
	InsetMathNest::metrics(mi);
	dim.asc = max(cell(0).ascent()  + 5, cell(1).ascent())  + 2;
	dim.des = max(cell(0).descent() - 5, cell(1).descent()) + 2;
	dim.wid = cell(0).width() + cell(1).width() + 10;
	metricsMarkers(dim);
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathRoot::draw(PainterInfo & pi, int x, int y) const
{
	int const w = cell(0).width();
	// the "exponent"
	cell(0).draw(pi, x, y - 5 - cell(0).descent());
	// the "base"
	cell(1).draw(pi, x + w + 8, y);
	Dimension const dim = dimension(*pi.base.bv);
	int const a = dim.ascent();
	int const d = dim.descent();
	int xp[4];
	int yp[4];
	pi.pain.line(x + dim.width(), y - a + 1,
							 x + w + 4, y - a + 1, Color::math);
	xp[0] = x + w + 4;         yp[0] = y - a + 1;
	xp[1] = x + w;             yp[1] = y + d;
	xp[2] = x + w - 2;         yp[2] = y + (d - a)/2 + 2;
	xp[3] = x + w - 5;         yp[3] = y + (d - a)/2 + 4;
	pi.pain.lines(xp, yp, 4, Color::math);
	drawMarkers(pi, x, y);
}


void InsetMathRoot::write(WriteStream & os) const
{
	os << "\\sqrt[" << cell(0) << "]{" << cell(1) << '}';
}


void InsetMathRoot::normalize(NormalStream & os) const
{
	os << "[root " << cell(0) << ' ' << cell(1) << ']';
}


bool InsetMathRoot::idxUpDown(Cursor & cur, bool up) const
{
	Cursor::idx_type const target = up ? 0 : 1;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = up ? cur.lastpos() : 0;
	return true;
}


void InsetMathRoot::maple(MapleStream & os) const
{
	os << '(' << cell(1) << ")^(1/(" << cell(0) <<"))";
}


void InsetMathRoot::mathematica(MathematicaStream & os) const
{
	os << '(' << cell(1) << ")^(1/(" << cell(0) <<"))";
}


void InsetMathRoot::octave(OctaveStream & os) const
{
	os << '(' << cell(1) << ")^(1/(" << cell(0) <<"))";
}


void InsetMathRoot::mathmlize(MathStream & os) const
{
	os << MTag("mroot") << cell(1) << cell(0) << ETag("mroot");
}


} // namespace lyx
