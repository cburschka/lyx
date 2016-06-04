/**
 * \file InsetMathXArrow.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/lassert.h"

#include "InsetMathXArrow.h"

#include "MathData.h"
#include "MathStream.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "LaTeXFeatures.h"
#include "MetricsInfo.h"


#include <algorithm>

using namespace std;

namespace lyx {


InsetMathXArrow::InsetMathXArrow(Buffer * buf, docstring const & name)
	: InsetMathFracBase(buf), name_(name)
{}


Inset * InsetMathXArrow::clone() const
{
	return new InsetMathXArrow(*this);
}


void InsetMathXArrow::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy = mi.base.changeScript();
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	dim.wid = max(dim0.width(), dim1.width()) + 10;
	dim.asc = dim0.height() + 10;
	dim.des = dim1.height();
	metricsMarkers(dim);
}


void InsetMathXArrow::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy = pi.base.changeScript();
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	// center the cells with the decoration
	cell(0).draw(pi, x + dim.width()/2 - dim0.width()/2, y - 10);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	cell(1).draw(pi, x + dim.width()/2 - dim1.width()/2, y + dim1.height());
	mathed_draw_deco(pi, x + 1, y - 7, dim.wid - 2, 5, name_);
	drawMarkers(pi, x, y);
}


void InsetMathXArrow::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << '\\' << name_;
	if (!cell(1).empty())
		os << '[' << cell(1) << ']';
	os << '{' << cell(0) << '}';
}


void InsetMathXArrow::normalize(NormalStream & os) const
{
	os << "[xarrow " << name_ << ' ' <<  cell(0) << ' ' << cell(1) << ']';
}


void InsetMathXArrow::mathmlize(MathStream & ms) const
{
	char const * arrow;
	
	if (name_ == "xleftarrow")
		arrow = "&larr;";
	else if (name_ == "xrightarrow")
		arrow = "&rarr;";
	else if (name_ == "xhookleftarrow")
		arrow = "&larrhk;";
	else if (name_ == "xhookrightarrow")
		arrow = "&rarrhk;";
	else if (name_ == "xLeftarrow")
		arrow = "&lArr;";
	else if (name_ == "xRightarrow")
		arrow = "&rArr;";
	else if (name_ == "xleftrightarrow")
		arrow = "&leftrightarrow;";
	else if (name_ == "xLeftrightarrow")
		arrow = "&Leftrightarrow;";
	else if (name_ == "xleftharpoondown")
		arrow = "&leftharpoondown;";
	else if (name_ == "xleftharpoonup")
		arrow = "&leftharpoonup;";
	else if (name_ == "xleftrightharpoons")
		arrow = "&leftrightharpoons;";
	else if (name_ == "xrightharpoondown")
		arrow = "&rightharpoondown;";
	else if (name_ == "xrightharpoonup")
		arrow = "&rightharpoonup;";
	else if (name_ == "xrightleftharpoons")
		arrow = "&rightleftharpoons;";
	else if (name_ == "xmapsto")
		arrow = "&mapsto;";
	else {
		lyxerr << "mathmlize conversion for '" << name_ << "' not implemented" << endl;
		LASSERT(false, arrow = "&rarr;");
	}
	ms << "<munderover accent='false' accentunder='false'>"
	   << arrow << cell(1) << cell(0)
	   << "</munderover>";
}


void InsetMathXArrow::htmlize(HtmlStream & os) const
{
	char const * arrow;

	if (name_ == "xleftarrow")
		arrow = "&larr;";
	else if (name_ == "xrightarrow")
		arrow = "&rarr;";
	else if (name_ == "xhookleftarrow")
		arrow = "&larrhk;";
	else if (name_ == "xhookrightarrow")
		arrow = "&rarrhk;";
	else if (name_ == "xLeftarrow")
		arrow = "&lArr;";
	else if (name_ == "xRightarrow")
		arrow = "&rArr;";
	else if (name_ == "xleftrightarrow")
		arrow = "&leftrightarrow;";
	else if (name_ == "xLeftrightarrow")
		arrow = "&Leftrightarrow;";
	else if (name_ == "xleftharpoondown")
		arrow = "&leftharpoondown;";
	else if (name_ == "xleftharpoonup")
		arrow = "&leftharpoonup;";
	else if (name_ == "xleftrightharpoons")
		arrow = "&leftrightharpoons;";
	else if (name_ == "xrightharpoondown")
		arrow = "&rightharpoondown;";
	else if (name_ == "xrightharpoonup")
		arrow = "&rightharpoonup;";
	else if (name_ == "xrightleftharpoons")
		arrow = "&rightleftharpoons;";
	else if (name_ == "xmapsto")
		arrow = "&mapsto;";
	else {
		lyxerr << "htmlize conversion for '" << name_ << "' not implemented" << endl;
		LASSERT(false, arrow = "&rarr;");
	}
	os << MTag("span", "class='xarrow'")
		 << MTag("span", "class='xatop'") << cell(0) << ETag("span")
		 << MTag("span", "class='xabottom'") << arrow << ETag("span")
		 << ETag("span");
}


void InsetMathXArrow::validate(LaTeXFeatures & features) const
{
	if (name_ == "xleftarrow" || name_ == "xrightarrow")
		features.require("amsmath");
	else
		features.require("mathtools");
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		// CSS adapted from eLyXer
		features.addCSSSnippet(
			"span.xarrow{display: inline-block; vertical-align: middle; text-align:center;}\n"
			"span.xatop{display: block;}\n"
			"span.xabottom{display: block;}");
	InsetMathNest::validate(features);
}


} // namespace lyx
