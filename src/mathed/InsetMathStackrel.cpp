/**
 * \file InsetMathStackrel.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathStackrel.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"

using namespace std;

namespace lyx {

InsetMathStackrel::InsetMathStackrel(Buffer * buf, bool sub)
	: InsetMathFracBase(buf, sub ? 3 : 2)
{}


Inset * InsetMathStackrel::clone() const
{
	return new InsetMathStackrel(*this);
}


bool InsetMathStackrel::idxUpDown(Cursor & cur, bool up) const
{
	if (up) {
		if (cur.idx() == 0)
			return false;
	} else {
		if (cur.idx() + 1 ==  nargs())
			return false;
	}
	InsetMath::idx_type target = up ? cur.idx() - 1 : cur.idx() + 1;
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(&cur.bv(), cur.x_target());
	return true;
}


void InsetMathStackrel::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	Changer dummy = mi.base.changeFrac();
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	if (nargs() > 2) {
		Dimension dim2;
		cell(2).metrics(mi, dim2);
		dim.wid = max(max(dim0.width(), dim1.width()), dim2.width()) + 4;
		dim.asc = dim1.ascent() + dim0.height() + 4;
		dim.des = dim1.descent() + dim2.height() + dim2.descent() + 1;
	} else {
		dim.wid = max(dim0.width(), dim1.width()) + 4;
		dim.asc = dim1.ascent() + dim0.height() + 4;
		dim.des = dim1.descent();
	}
	metricsMarkers(dim);
}


void InsetMathStackrel::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	int m  = x + dim.width() / 2;
	int yo = y - dim1.ascent() - dim0.descent() - 1;
	cell(1).draw(pi, m - dim1.width() / 2, y);
	Changer dummy = pi.base.changeFrac();
	cell(0).draw(pi, m - dim0.width() / 2, yo);
	if (nargs() > 2) {
		Dimension const & dim2 = cell(2).dimension(*pi.base.bv);
		int y2 = y + dim1.descent() + dim2.ascent() + 1;
		cell(2).draw(pi, m - dim2.width() / 2, y2);
	}
	drawMarkers(pi, x, y);
}


void InsetMathStackrel::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\stackrel";
	if (nargs() > 2)
		os << '[' << cell(2) << ']';
	os << '{' << cell(0) << "}{" << cell(1) << '}';
}


void InsetMathStackrel::normalize(NormalStream & os) const
{
	os << "[stackrel " << cell(0) << ' ' << cell(1);
	if (nargs() > 2)
		os << ' ' << cell(2);
	os << ']';
}


void InsetMathStackrel::mathmlize(MathStream & ms) const
{
	if (nargs() > 2)
		ms << "<munderover>" << cell(1) << cell(2) << cell(0) << "</munderover>";
	else
		ms << "<mover accent='false'>" << cell(1) << cell(0) << "</mover>";
}


void InsetMathStackrel::htmlize(HtmlStream & os) const
{
	if (nargs() > 2) {
		os << MTag("span", "class='underoverset'")
		   << MTag("span", "class='top'") << cell(0) << ETag("span")
		   << MTag("span") << cell(1) << ETag("span")
		   << MTag("span", "class='bottom'") << cell(2) << ETag("span");
	} else {
		// at the moment, this is exactly the same as overset
		os << MTag("span", "class='overset'")
		   << MTag("span", "class='top'") << cell(0) << ETag("span")
		   << MTag("span") << cell(1) << ETag("span");
	}
	os << ETag("span");
}


void InsetMathStackrel::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML) {
		if (nargs() > 2) {
			// FIXME: "vertical-align: middle" works only if the
			// height of sub and super script is approximately equal.
			features.addCSSSnippet(
				"span.underoverset{display: inline-block; vertical-align: middle; text-align:center;}\n"
				"span.underoverset span {display: block;}\n"
				"span.bottom{font-size: 66%;}\n"
				"span.top{font-size: 66%;}");
		} else {
			// from overset
			features.addCSSSnippet(
				"span.overset{display: inline-block; vertical-align: bottom; text-align:center;}\n"
				"span.overset span {display: block;}\n"
				"span.top{font-size: 66%;}");
		}
	}
	if (nargs() > 2)
		features.require("stackrel");

	InsetMathNest::validate(features);
}

} // namespace lyx
