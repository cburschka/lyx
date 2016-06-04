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
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "frontends/Painter.h"


using namespace std;

namespace lyx {


InsetMathRoot::InsetMathRoot(Buffer * buf)
	: InsetMathNest(buf, 2)
{}


Inset * InsetMathRoot::clone() const
{
	return new InsetMathRoot(*this);
}


void InsetMathRoot::metrics(MetricsInfo & mi, Dimension & dim) const
{
	InsetMathNest::metrics(mi);
	Dimension const & dim0 = cell(0).dimension(*mi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*mi.base.bv);
	dim.asc = max(dim0.ascent()  + 5, dim1.ascent())  + 2;
	dim.des = max(dim0.descent() - 5, dim1.descent()) + 2;
	dim.wid = dim0.width() + dim1.width() + 10;
	metricsMarkers(dim);
}


void InsetMathRoot::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	int const w = dim0.width();
	// the "exponent"
	cell(0).draw(pi, x, y - 5 - dim0.descent());
	// the "base"
	cell(1).draw(pi, x + w + 8, y);
	Dimension const dim = dimension(*pi.base.bv);
	int const a = dim.ascent();
	int const d = dim.descent();
	int xp[4];
	int yp[4];
	pi.pain.line(x + dim.width(), y - a + 1,
				x + w + 4, y - a + 1, pi.base.font.color());
	xp[0] = x + w + 4;         yp[0] = y - a + 1;
	xp[1] = x + w;             yp[1] = y + d;
	xp[2] = x + w - 2;         yp[2] = y + (d - a)/2 + 2;
	xp[3] = x + w - 5;         yp[3] = y + (d - a)/2 + 4;
	pi.pain.lines(xp, yp, 4, pi.base.font.color());
	drawMarkers(pi, x, y);
}


void InsetMathRoot::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
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


void InsetMathRoot::htmlize(HtmlStream & os) const
{
	os << MTag("span", "class='root'")
	   << MTag("sup") << cell(0) << ETag("sup")
	   << from_ascii("&radic;") 
	   << MTag("span", "class='rootof'")	<< cell(1) << ETag("span") 
		 << ETag("span");
}


void InsetMathRoot::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.rootof{border-top: thin solid black;}\n"
			"span.root sup{font-size: 75%;}");
	InsetMathNest::validate(features);
}

} // namespace lyx
