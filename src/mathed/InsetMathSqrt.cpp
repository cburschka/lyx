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
#include "TextPainter.h"
#include "frontends/Painter.h"


namespace lyx {

InsetMathSqrt::InsetMathSqrt()
	: InsetMathNest(1)
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
		x + 8, y - a + 1, Color_math);
	xp[0] = x + 8;            yp[0] = y - a + 1;
	xp[1] = x + 5;            yp[1] = y + d - 1;
	xp[2] = x;                yp[2] = y + (d - a)/2;
	pi.pain.lines(xp, yp, 3, Color_math);
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
	bool brace = ensureMath(os);
	os << "\\sqrt{" << cell(0) << '}';
	os.pendingBrace(brace);
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


} // namespace lyx
