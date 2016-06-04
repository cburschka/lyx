/**
 * \file InsetMathSqrt.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathSqrt.h"

#include "MathData.h"
#include "MathStream.h"

#include "LaTeXFeatures.h"
#include "MetricsInfo.h"
#include "TextPainter.h"

#include "frontends/Painter.h"


namespace lyx {

InsetMathSqrt::InsetMathSqrt(Buffer * buf)
	: InsetMathNest(buf, 1)
{}


Inset * InsetMathSqrt::clone() const
{
	return new InsetMathSqrt(*this);
}


void InsetMathSqrt::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	dim.asc += 4;
	dim.des += 2;
	dim.wid += 12;
	metricsMarkers(dim);
}


void InsetMathSqrt::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 10, y);
	Dimension const dim = dimension(*pi.base.bv);
	int const a = dim.ascent();
	int const d = dim.descent();
	int xp[3];
	int yp[3];
	pi.pain.line(x + dim.width(), y - a + 1,
		x + 8, y - a + 1, pi.base.font.color());
	xp[0] = x + 8;            yp[0] = y - a + 1;
	xp[1] = x + 5;            yp[1] = y + d - 1;
	xp[2] = x;                yp[2] = y + (d - a)/2;
	pi.pain.lines(xp, yp, 3, pi.base.font.color());
	drawMarkers(pi, x, y);
}


void InsetMathSqrt::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
	dim.asc += 1;
	dim.wid += 2;
}


void InsetMathSqrt::drawT(TextPainter & /*pain*/, int /*x*/, int /*y*/) const
{
	/*
	cell(0).drawT(pain, x + 2, y);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	pain.horizontalLine(x + 2, y - dim0.ascent(), dim0.width(), '_');
	pain.verticalLine  (x + 1, y - dim0.ascent() + 1, dim0.height());
	pain.draw(x, y + dim0.descent(), '\\');
	*/
}


void InsetMathSqrt::write(WriteStream & os) const
{
	MathEnsurer ensurer(os);
	os << "\\sqrt{" << cell(0) << '}';
}


void InsetMathSqrt::normalize(NormalStream & os) const
{
	os << "[sqrt " << cell(0) << ']';
}

void InsetMathSqrt::maple(MapleStream & os) const
{
	os << "sqrt(" << cell(0) << ')';
}

void InsetMathSqrt::mathematica(MathematicaStream & os) const
{
	os << "Sqrt[" << cell(0) << ']';
}


void InsetMathSqrt::octave(OctaveStream & os) const
{
	os << "sqrt(" << cell(0) << ')';
}


void InsetMathSqrt::mathmlize(MathStream & os) const
{
	os << MTag("msqrt") << cell(0) << ETag("msqrt");
}


void InsetMathSqrt::htmlize(HtmlStream & os) const
{
	os << MTag("span", "class='sqrt'")
	   << from_ascii("&radic;") 
	   << MTag("span", "class='sqrtof'")	<< cell(0) << ETag("span") 
		 << ETag("span");
}


void InsetMathSqrt::validate(LaTeXFeatures & features) const
{
	if (features.runparams().math_flavor == OutputParams::MathAsHTML)
		features.addCSSSnippet("span.sqrtof{border-top: thin solid black;}");
	InsetMathNest::validate(features);
}

} // namespace lyx
