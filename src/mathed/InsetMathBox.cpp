/**
 * \file InsetMathBox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Ling Li (InsetMathMakebox)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBox.h"

#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"

#include "support/gettext.h"
#include "support/lstrings.h"

#include "frontends/Painter.h"

#include <algorithm>
#include <ostream>

using namespace lyx::support;

namespace lyx {

/////////////////////////////////////////////////////////////////////
//
// InsetMathBox
//
/////////////////////////////////////////////////////////////////////

InsetMathBox::InsetMathBox(Buffer * buf, docstring const & name)
	: InsetMathNest(buf, 1), name_(name)
{}


void InsetMathBox::write(WriteStream & os) const
{
	ModeSpecifier specifier(os, TEXT_MODE);
	os << '\\' << name_ << '{' << cell(0) << '}';
}


void InsetMathBox::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ';
	//text_->write(buffer(), os);
	os << "] ";
}


void InsetMathBox::mathmlize(MathStream & ms) const
{
	// FIXME XHTML
	// Need to do something special for tags here.
	// Probably will have to involve deferring them, which
	// means returning something from this routine.
	SetMode textmode(ms, true);
	ms << MTag("mstyle", "class='mathbox'")
	   << cell(0)
	   << ETag("mstyle");
}


void InsetMathBox::htmlize(HtmlStream & ms) const
{
	SetHTMLMode textmode(ms, true);
	ms << MTag("span", "class='mathbox'")
	   << cell(0)
	   << ETag("span");
}


void InsetMathBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathBox::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	cell(0).draw(pi, x, y);
	drawMarkers(pi, x, y);
}


void InsetMathBox::infoize(odocstream & os) const
{	
	os << bformat(_("Box: %1$s"), name_);
}


void InsetMathBox::validate(LaTeXFeatures & features) const
{
	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().math_flavor == OutputParams::MathAsMathML)
		features.addCSSSnippet("mstyle.mathbox { font-style: normal; }");
	else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet("span.mathbox { font-style: normal; }");

	if (name_ == "tag" || name_ == "tag*")
		features.require("amsmath");

	InsetMathNest::validate(features);
}



/////////////////////////////////////////////////////////////////////
//
// InsetMathFBox
//
/////////////////////////////////////////////////////////////////////


InsetMathFBox::InsetMathFBox(Buffer * buf)
	: InsetMathNest(buf, 1)
{}


void InsetMathFBox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers2(dim, 3); // 1 pixel space, 1 frame, 1 space
}


void InsetMathFBox::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
		dim.width() - 2, dim.height() - 2, Color_foreground);
	FontSetChanger dummy(pi.base, "textnormal");
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void InsetMathFBox::write(WriteStream & os) const
{
	ModeSpecifier specifier(os, TEXT_MODE);
	os << "\\fbox{" << cell(0) << '}';
}


void InsetMathFBox::normalize(NormalStream & os) const
{
	os << "[fbox " << cell(0) << ']';
}


void InsetMathFBox::mathmlize(MathStream & ms) const
{	
	SetMode textmode(ms, true);
	ms << MTag("mstyle", "class='fbox'")
	   << cell(0)
	   << ETag("mstyle");
}


void InsetMathFBox::htmlize(HtmlStream & ms) const
{
	SetHTMLMode textmode(ms, true);
	ms << MTag("span", "class='fbox'")
	   << cell(0)
	   << ETag("span");
}


void InsetMathFBox::infoize(odocstream & os) const
{
	os << "FBox: ";
}


void InsetMathFBox::validate(LaTeXFeatures & features) const
{
	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().math_flavor == OutputParams::MathAsMathML)
		features.addCSSSnippet(
			"mstyle.fbox { border: 1px solid black; font-style: normal; padding: 0.5ex; }");
	else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet(
			"span.fbox { border: 1px solid black; font-style: normal; padding: 0.5ex; }");

	cell(0).validate(features);
	InsetMathNest::validate(features);
}



/////////////////////////////////////////////////////////////////////
//
// InsetMathMakebox
//
/////////////////////////////////////////////////////////////////////


InsetMathMakebox::InsetMathMakebox(Buffer * buf, bool framebox)
	: InsetMathNest(buf, 3), framebox_(framebox)
{}


void InsetMathMakebox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	
	Dimension wdim;
	static docstring bracket = from_ascii("[");
	metricsStrRedBlack(mi, wdim, bracket);
	int w = wdim.wid;
	
	Dimension dim0;
	Dimension dim1;
	Dimension dim2;
	cell(0).metrics(mi, dim0);
	cell(1).metrics(mi, dim1);
	cell(2).metrics(mi, dim2);
	
	dim.wid = w + dim0.wid + w + w + dim1.wid + w + 2 + dim2.wid;
	dim.asc = std::max(std::max(wdim.asc, dim0.asc), std::max(dim1.asc, dim2.asc)); 
	dim.des = std::max(std::max(wdim.des, dim0.des), std::max(dim1.des, dim2.des));
	
	if (framebox_) {
		dim.wid += 4;
		dim.asc += 3;
		dim.des += 2;
	} else {
		dim.asc += 1;
		dim.des += 1;
	}
	
	metricsMarkers(dim);
}


void InsetMathMakebox::draw(PainterInfo & pi, int x, int y) const
{
	drawMarkers(pi, x, y);
	
	FontSetChanger dummy(pi.base, "textnormal");
	BufferView const & bv = *pi.base.bv;
	int w = mathed_char_width(pi.base.font, '[');
	
	if (framebox_) {
		Dimension const dim = dimension(*pi.base.bv);
		pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
				  dim.width() - 2, dim.height() - 2, Color_foreground);
		x += 2;
	}
	
	drawStrBlack(pi, x, y, from_ascii("["));
	x += w;
	cell(0).draw(pi, x, y);
	x += cell(0).dimension(bv).wid;
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w;
	cell(1).draw(pi, x, y);
	x += cell(1).dimension(bv).wid;
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w + 2;

	cell(2).draw(pi, x, y);
}


void InsetMathMakebox::write(WriteStream & os) const
{
	ModeSpecifier specifier(os, TEXT_MODE);
	os << (framebox_ ? "\\framebox" : "\\makebox");
	if (!cell(0).empty() || !os.latex()) {
		os << '[' << cell(0) << ']';
		if (!cell(1).empty() || !os.latex())
			os << '[' << cell(1) << ']';
	}
	os << '{' << cell(2) << '}';
}


void InsetMathMakebox::normalize(NormalStream & os) const
{
	os << (framebox_ ? "[framebox " : "[makebox ")
	   << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}


void InsetMathMakebox::infoize(odocstream & os) const
{
	os << (framebox_ ? "Framebox" : "Makebox") 
	   << " (width: " << cell(0)
	   << " pos: " << cell(1) << ")";
}


void InsetMathMakebox::mathmlize(MathStream & ms) const
{
	// FIXME We could do something with the other arguments.
	std::string const cssclass = framebox_ ? "framebox" : "makebox";
	SetMode textmode(ms, true);
	ms << MTag("mstyle", "class='" + cssclass + "'")
	   << cell(2)
	   << ETag("mstyle");
}


void InsetMathMakebox::htmlize(HtmlStream & ms) const
{
	// FIXME We could do something with the other arguments.
	SetHTMLMode textmode(ms, true);
	std::string const cssclass = framebox_ ? "framebox" : "makebox";
	ms << MTag("span", "class='" + cssclass + "'")
	   << cell(2)
	   << ETag("span");
}


void InsetMathMakebox::validate(LaTeXFeatures & features) const
{
	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().math_flavor == OutputParams::MathAsMathML)
		features.addCSSSnippet("mstyle.framebox { border: 1px solid black; }");
	else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet("span.framebox { border: 1px solid black; }");
	InsetMathNest::validate(features);
}


/////////////////////////////////////////////////////////////////////
//
// InsetMathBoxed
//
/////////////////////////////////////////////////////////////////////

InsetMathBoxed::InsetMathBoxed(Buffer * buf)
	: InsetMathNest(buf, 1)
{}


void InsetMathBoxed::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers2(dim, 3); // 1 pixel space, 1 frame, 1 space
}


void InsetMathBoxed::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	pi.pain.rectangle(x + 1, y - dim.ascent() + 1,
		dim.width() - 2, dim.height() - 2, Color_foreground);
	cell(0).draw(pi, x + 3, y);
	setPosCache(pi, x, y);
}


void InsetMathBoxed::write(WriteStream & os) const
{
	ModeSpecifier specifier(os, MATH_MODE);
	os << "\\boxed{" << cell(0) << '}';
}


void InsetMathBoxed::normalize(NormalStream & os) const
{
	os << "[boxed " << cell(0) << ']';
}


void InsetMathBoxed::infoize(odocstream & os) const
{
	os << "Boxed: ";
}


void InsetMathBoxed::mathmlize(MathStream & ms) const
{
	ms << MTag("mstyle", "class='boxed'")
	   << cell(0)
	   << ETag("mstyle");
}


void InsetMathBoxed::htmlize(HtmlStream & ms) const
{
	ms << MTag("span", "class='boxed'")
	   << cell(0)
		 << ETag("span");
}


void InsetMathBoxed::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");

	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().math_flavor == OutputParams::MathAsMathML)
		features.addCSSSnippet("mstyle.boxed { border: 1px solid black; }");
	else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet("span.boxed { border: 1px solid black; }");
	
	InsetMathNest::validate(features);
}


} // namespace lyx
