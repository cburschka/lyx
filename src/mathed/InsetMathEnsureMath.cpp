/**
 * \file InsetMathEnsureMath.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathEnsureMath.h"

#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "LaTeXFeatures.h"
#include "MetricsInfo.h"

#include <ostream>


namespace lyx {

InsetMathEnsureMath::InsetMathEnsureMath(Buffer * buf)
	: InsetMathNest(buf, 1)
{}


Inset * InsetMathEnsureMath::clone() const
{
	return new InsetMathEnsureMath(*this);
}


void InsetMathEnsureMath::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Changer dummy = mi.base.changeEnsureMath();
	cell(0).metrics(mi, dim);
}


void InsetMathEnsureMath::draw(PainterInfo & pi, int x, int y) const
{
	Changer dummy = pi.base.changeEnsureMath();
	cell(0).draw(pi, x, y);
}


void InsetMathEnsureMath::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
}


void InsetMathEnsureMath::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void InsetMathEnsureMath::write(WriteStream & os) const
{
	ModeSpecifier specifier(os, MATH_MODE);
	os << "\\ensuremath{" << cell(0) << "}";
}


void InsetMathEnsureMath::mathmlize(MathMLStream & ms) const
{
	SetMode mathmode(ms, false);
	ms << MTag("mstyle", "class='math'")
	   << cell(0)
	   << ETag("mstyle");
}


void InsetMathEnsureMath::htmlize(HtmlStream & os) const
{
	SetHTMLMode mathmode(os, false);
	os << MTag("span", "class='math'")
	   << cell(0)
	   << ETag("span");
}


void InsetMathEnsureMath::infoize(odocstream & os) const
{
	os << "EnsureMath";
}


void InsetMathEnsureMath::validate(LaTeXFeatures & features) const
{
	// FIXME XHTML
	// It'd be better to be able to get this from an InsetLayout, but at present
	// InsetLayouts do not seem really to work for things that aren't InsetTexts.
	if (features.runparams().math_flavor == OutputParams::MathAsMathML)
		features.addCSSSnippet("mstyle.math { font-style: italic; }");
	else if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet("span.mathbox { font-style: italic; }");

	InsetMathNest::validate(features);
}

} // namespace lyx
