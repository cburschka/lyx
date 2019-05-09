/**
 * \file InsetMathOverset.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathOverset.h"

#include "MathData.h"
#include "MathStream.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "support/lassert.h"

using namespace std;

namespace lyx {

Inset * InsetMathOverset::clone() const
{
	return new InsetMathOverset(*this);
}


void InsetMathOverset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy2 = mi.base.changeEnsureMath();
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Changer dummy = mi.base.changeScript();
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	dim.wid = max(dim1.width(), dim0.wid) + 4;
	dim.asc = dim0.asc + dim1.height() + 4;
	dim.des = dim0.des;
}


void InsetMathOverset::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy2 = pi.base.changeEnsureMath();
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	int m  = x + dim.wid / 2;
	int yo = y - dim0.asc - dim1.des - 1;
	cell(0).draw(pi, m - dim0.wid / 2, y);
	Changer dummy = pi.base.changeScript();
	cell(1).draw(pi, m - dim1.width() / 2, yo);
}


bool InsetMathOverset::idxUpDown(Cursor & cur, bool up) const
{
	idx_type target = up; // up ? 1 : 0, since upper cell has idx 1
	if (cur.idx() == target)
		return false;
	cur.idx() = target;
	cur.pos() = cur.cell().x2pos(&cur.bv(), cur.x_target());
	return true;
}


bool InsetMathOverset::idxFirst(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 0;
	cur.pos() = 0;
	return true;
}


bool InsetMathOverset::idxLast(Cursor & cur) const
{
	LASSERT(&cur.inset() == this, return false);
	cur.idx() = 0;
	cur.pos() = cur.lastpos();
	return true;
}


void InsetMathOverset::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	if (os.fragile())
		os << "\\protect";
	os << "\\overset{" << cell(1) << "}{" << cell(0) << '}';
}


void InsetMathOverset::normalize(NormalStream & os) const
{
	os << "[overset " << cell(1) << ' ' << cell(0) << ']';
}


void InsetMathOverset::mathmlize(MathStream & ms) const
{
	ms << "<" << from_ascii(ms.namespacedTag("mover")) << " accent='false'>"
	   << cell(0) << cell(1)
	   << "</" << from_ascii(ms.namespacedTag("mover")) << ">";
}


void InsetMathOverset::htmlize(HtmlStream & os) const
{
	os << MTag("span", "class='overset'")
	   << MTag("span", "class='top'") << cell(1) << ETag("span")
	   << MTag("span") << cell(0) << ETag("span")
	   << ETag("span");
}


void InsetMathOverset::validate(LaTeXFeatures & features) const
{
	if (features.runparams().isLaTeX())
		features.require("amsmath");
	else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.overset{display: inline-block; vertical-align: bottom; text-align:center;}\n"
			"span.overset span {display: block;}\n"
			"span.top{font-size: 66%;}");

	InsetMathNest::validate(features);
}


} // namespace lyx
