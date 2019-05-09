/**
 * \file InsetMathUnderset.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathUnderset.h"

#include "MathData.h"
#include "MathStream.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {

Inset * InsetMathUnderset::clone() const
{
	return new InsetMathUnderset(*this);
}


void InsetMathUnderset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy2 = mi.base.changeEnsureMath();
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Changer dummy = mi.base.changeScript();
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	dim.wid = max(dim1.width(), dim0.width()) + 4;
	dim.asc = dim0.ascent();
	dim.des = dim0.descent() + dim1.height() + 4;
}


void InsetMathUnderset::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy2 = pi.base.changeEnsureMath();
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	int m  = x + dim.wid / 2;
	int yo = y + dim0.descent() + dim1.ascent() + 1;
	cell(0).draw(pi, m - dim0.width() / 2, y);
	Changer dummy = pi.base.changeScript();
	cell(1).draw(pi, m - dim1.width() / 2, yo);
}


bool InsetMathUnderset::idxFirst(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool InsetMathUnderset::idxLast(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 0;
	cur.pos() = cur.lastpos();
	return true;
}


void InsetMathUnderset::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	if (os.fragile())
		os << "\\protect";
	os << "\\underset{" << cell(1) << "}{" << cell(0) << '}';
}


void InsetMathUnderset::normalize(NormalStream & os) const
{
	os << "[underset " << cell(1) << ' ' << cell(0) << ']';
}


void InsetMathUnderset::mathmlize(MathStream & ms) const
{
	ms << "<" << from_ascii(ms.namespacedTag("munder")) << " accent='false'>"
	   << cell(0) << cell(1)
	   << "</" << from_ascii(ms.namespacedTag("munder")) << ">";
}


void InsetMathUnderset::htmlize(HtmlStream & os) const
{
	os << MTag("span", "class='underset'")
		 << MTag("span") << cell(1) << ETag("span")
		 << MTag("span", "class='bottom'") << cell(0) << ETag("span")
		 << ETag("span");
}


void InsetMathUnderset::validate(LaTeXFeatures & features) const
{
	if (features.runparams().isLaTeX())
		features.require("amsmath");
	else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.underset{display: inline-block; vertical-align: top; text-align:center;}\n"
			"span.underset span {display: block;}\n"
			"span.bottom{font-size: 66%;}");

	InsetMathNest::validate(features);
}


} // namespace lyx
