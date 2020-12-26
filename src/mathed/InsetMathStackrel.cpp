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

#include "MathData.h"
#include "MathStream.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "support/lassert.h"

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
	idx_type const npos = 1234; // impossible number
	idx_type target = npos;
	if (up) {
		idx_type const targets[] = { 1, npos, 0 };
		target = targets[cur.idx()];
	} else {
		idx_type const targets[] = { 2, 0, npos };
		target = targets[cur.idx()];
	}

	if (target == npos || target == nargs())
		return false;
	cur.idx() = target;
	cur.pos() = cell(target).x2pos(&cur.bv(), cur.x_target());
	return true;
}


bool InsetMathStackrel::idxFirst(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool InsetMathStackrel::idxLast(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 0;
	cur.pos() = cur.lastpos();
	return true;
}


MathClass InsetMathStackrel::mathClass() const
{
	// FIXME: update this when/if \stackbin is supported
	return MC_REL;
}


void InsetMathStackrel::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy2 = mi.base.changeEnsureMath();
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Changer dummy = mi.base.changeScript();
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	if (nargs() > 2) {
		Dimension dim2;
		cell(2).metrics(mi, dim2);
		dim.wid = max(max(dim1.width(), dim0.width()), dim2.width()) + 4;
		dim.asc = dim0.ascent() + dim1.height() + 4;
		dim.des = dim0.descent() + dim2.height() + dim2.descent() + 1;
	} else {
		dim.wid = max(dim1.width(), dim0.width()) + 4;
		dim.asc = dim0.ascent() + dim1.height() + 4;
		dim.des = dim0.descent();
	}
}


void InsetMathStackrel::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy2 = pi.base.changeEnsureMath();
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	int m  = x + dim.width() / 2;
	int yo = y - dim0.ascent() - dim1.descent() - 1;
	cell(0).draw(pi, m - dim0.width() / 2, y);
	Changer dummy = pi.base.changeScript();
	cell(1).draw(pi, m - dim1.width() / 2, yo);
	if (nargs() > 2) {
		Dimension const & dim2 = cell(2).dimension(*pi.base.bv);
		int y2 = y + dim0.descent() + dim2.ascent() + 1;
		cell(2).draw(pi, m - dim2.width() / 2, y2);
	}
}


void InsetMathStackrel::write(TeXMathStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\stackrel";
	if (nargs() > 2)
		os << '[' << cell(2) << ']';
	os << '{' << cell(1) << "}{" << cell(0) << '}';
}


void InsetMathStackrel::normalize(NormalStream & os) const
{
	os << "[stackrel " << cell(1) << ' ' << cell(0);
	if (nargs() > 2)
		os << ' ' << cell(2);
	os << ']';
}


void InsetMathStackrel::mathmlize(MathMLStream & ms) const
{
	if (nargs() > 2)
		ms << "<" << from_ascii(ms.namespacedTag("munderover")) << ">"
		   << cell(0) << cell(2) << cell(1)
		   << "</" << from_ascii(ms.namespacedTag("munderover")) << ">";
	else
		ms << "<" << from_ascii(ms.namespacedTag("mover")) << " accent='false'>"
		   << cell(0) << cell(1)
		   << "</" << from_ascii(ms.namespacedTag("mover")) << ">";
}


void InsetMathStackrel::htmlize(HtmlStream & os) const
{
	if (nargs() > 2) {
		os << MTag("span", "class='underoverset'")
		   << MTag("span", "class='top'") << cell(1) << ETag("span")
		   << MTag("span") << cell(0) << ETag("span")
		   << MTag("span", "class='bottom'") << cell(2) << ETag("span");
	} else {
		// at the moment, this is exactly the same as overset
		os << MTag("span", "class='overset'")
		   << MTag("span", "class='top'") << cell(1) << ETag("span")
		   << MTag("span") << cell(0) << ETag("span");
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
