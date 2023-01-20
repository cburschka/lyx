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

#include "InsetMathXArrow.h"

#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include "support/debug.h"
#include "support/lassert.h"

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
	Changer dummy2 = mi.base.changeEnsureMath();
	Changer dummy = mi.base.changeScript();
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	dim.wid = max(dim0.width(), dim1.width()) + 10;
	dim.asc = dim0.height() + 10;
	dim.des = dim1.height();
}


void InsetMathXArrow::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy2 = pi.base.changeEnsureMath();
	Changer dummy = pi.base.changeScript();
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	// center the cells with the decoration
	cell(0).draw(pi, x + dim.width()/2 - dim0.width()/2, y - 10);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	cell(1).draw(pi, x + dim.width()/2 - dim1.width()/2, y + dim1.height());
	mathed_draw_deco(pi, x, y - 7, dim.wid, 5, name_);
}


void InsetMathXArrow::write(TeXMathStream & os) const
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


static std::map<string, string> latex_to_xml_entities = {
		{"xleftarrow", "&#x2190;"},
		{"xrightarrow", "&#x2192;"},
		{"xhookleftarrow", "&#x21a9;"},
		{"xhookrightarrow", "&#x21aa;"},
		{"xLeftarrow", "&#x21d0;"},
		{"xRightarrow", "&#x21d2;"},
		{"xleftrightarrow", "&#x2194;"},
		{"xLeftrightarrow", "&#x21d4;"},
		{"xleftharpoondown", "&#x21bd;"},
		{"xleftharpoonup", "&#x21bc;"},
		{"xleftrightharpoons", "&#x21cb;"},
		{"xrightharpoondown", "&#x21c1;"},
		{"xrightharpoonup", "&#x21c0;"},
		{"xrightleftharpoons", "&#x21cc;"},
		{"xmapsto", "&#x21a6;"},
};


docstring map_latex_to(docstring latex)
{
	auto mapping = latex_to_xml_entities.find(to_ascii(latex));
	if (mapping != latex_to_xml_entities.end()) {
		return from_ascii(mapping->second);
	} else {
		lyxerr << "mathmlize conversion for '" << latex << "' not implemented" << endl;
		LASSERT(false, return from_ascii(latex_to_xml_entities["xrightarrow"]));
		return docstring();
	}
}


void InsetMathXArrow::mathmlize(MathMLStream & ms) const
{
	docstring arrow = map_latex_to(name_);
	ms << MTag("munderover", "accent='false' accentunder='false'")
	   << MTagInline("mo") << arrow << ETagInline("mo")
	   << cell(1) << cell(0)
	   << ETag("munderover");
}


void InsetMathXArrow::htmlize(HtmlStream & os) const
{
	docstring arrow = map_latex_to(name_);
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
