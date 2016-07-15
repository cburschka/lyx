/**
 * \file InsetMathBig.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBig.h"

#include "LaTeXFeatures.h"
#include "MathSupport.h"
#include "MathStream.h"
#include "MetricsInfo.h"

#include "frontends/FontMetrics.h"

#include "support/docstream.h"
#include "support/lstrings.h"


namespace lyx {


InsetMathBig::InsetMathBig(docstring const & name, docstring const & delim)
	: name_(name), delim_(delim)
{}


docstring InsetMathBig::name() const
{
	return name_;
}


Inset * InsetMathBig::clone() const
{
	return new InsetMathBig(*this);
}


InsetMathBig::size_type InsetMathBig::size() const
{
	// order: big Big bigg Bigg biggg Biggg
	//        0   1   2    3    4     5
	char_type const c = name_[name_.size() - 1];
	int const base_size = (c == 'l' || c == 'm' || c == 'r') ? 4 : 3;
	return name_[0] == 'B' ?
		2 * (name_.size() - base_size) + 1:
		2 * (name_.size() - base_size);
}


double InsetMathBig::increase() const
{
	// The formula used in amsmath.sty is
	// 1.2 * (1.0 + size() * 0.5) - 1.0.
	// We use a smaller step and a bigger offset because our base size
	// is different.
	return (size() + 1) * 0.3;
}


void InsetMathBig::metrics(MetricsInfo & mi, Dimension & dim) const
{
	double const h = theFontMetrics(mi.base.font).ascent('I');
	double const f = increase();
	dim.wid = 6;
	dim.asc = int(h + f * h);
	dim.des = int(f * h);
}


void InsetMathBig::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	// mathed_draw_deco does not use the leading backslash, so remove it
	// (but don't use ltrim if this is the backslash delimiter).
	// Replace \| by \Vert (equivalent in LaTeX), since mathed_draw_deco
	// would treat it as |.
	docstring const delim = (delim_ == "\\|") ?  from_ascii("Vert") :
		(delim_ == "\\\\") ? from_ascii("\\") : support::ltrim(delim_, "\\");
	mathed_draw_deco(pi, x + 1, y - dim.ascent(), 4, dim.height(),
			 delim);
	setPosCache(pi, x, y);
}


void InsetMathBig::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << '\\' << name_ << delim_;
	if (delim_[0] == '\\')
		os.pendingSpace(true);
}


void InsetMathBig::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ' << delim_ << ']';
}


void InsetMathBig::mathmlize(MathStream & os) const
{
	os << "<mo form='prefix' fence='true' stretchy='true' symmetric='true'>"
	   << convertDelimToXMLEscape(delim_)
	   << "</mo>";
}


void InsetMathBig::htmlize(HtmlStream & os) const
{
	std::string name;
	switch (size()) {
	case 0: case 1: name = "big"; break;
	case 2: case 3: name = "bigg"; break;
	case 4: case 5: name = "biggg"; break;
	default: name  = "big"; break;
	}
	os << MTag("span", "class='" + name + "symbol'")
	   << convertDelimToXMLEscape(delim_)
	   << ETag("span");
}


void InsetMathBig::infoize2(odocstream & os) const
{
	os << name_;
}


bool InsetMathBig::isBigInsetDelim(docstring const & delim)
{
	// mathed_draw_deco must handle these
	static char const * const delimiters[] = {
		"(", ")", "\\{", "\\}", "\\lbrace", "\\rbrace", "[", "]",
		"|", "/", "\\slash", "\\|", "\\vert", "\\Vert", "'",
		"<", ">", "\\\\", "\\backslash",
		"\\langle", "\\lceil", "\\lfloor",
		"\\rangle", "\\rceil", "\\rfloor",
		"\\downarrow", "\\Downarrow",
		"\\uparrow", "\\Uparrow",
		"\\updownarrow", "\\Updownarrow", ""
	};
	return support::findToken(delimiters, to_utf8(delim)) >= 0;
}


void InsetMathBig::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.bigsymbol{font-size: 150%;}\n"
			"span.biggsymbol{font-size: 200%;}\n"
			"span.bigggsymbol{font-size: 225%;}");
}


} // namespace lyx
