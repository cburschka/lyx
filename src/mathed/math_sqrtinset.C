/**
 * \file math_sqrtinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_sqrtinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "textpainter.h"
#include "frontends/Painter.h"

using std::auto_ptr;


MathSqrtInset::MathSqrtInset()
	: MathNestInset(1)
{}


auto_ptr<InsetBase> MathSqrtInset::clone() const
{
	return auto_ptr<InsetBase>(new MathSqrtInset(*this));
}


void MathSqrtInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim_);
	dim_.asc += 4;
	dim_.des += 2;
	dim_.wid += 12;
	metricsMarkers(1);
	dim = dim_;
}


void MathSqrtInset::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 10, y);
	int const a = dim_.ascent();
	int const d = dim_.descent();
	int xp[4];
	int yp[4];
	xp[0] = x + dim_.width(); yp[0] = y - a + 1;
	xp[1] = x + 8;            yp[1] = y - a + 1;
	xp[2] = x + 5;            yp[2] = y + d - 1;
	xp[3] = x;                yp[3] = y + (d - a)/2;
	pi.pain.lines(xp, yp, 4, LColor::math);
	drawMarkers(pi, x, y);
}


void MathSqrtInset::metricsT(TextMetricsInfo const & mi, Dimension & dim) const
{
	cell(0).metricsT(mi, dim);
	dim.asc += 1;
	dim.wid += 2;
}


void MathSqrtInset::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x + 2, y);
	pain.horizontalLine(x + 2, y - cell(0).ascent(), cell(0).width(), '_');
	pain.verticalLine  (x + 1, y - cell(0).ascent() + 1, cell(0).height());
	pain.draw(x, y + cell(0).descent(), '\\');
}


void MathSqrtInset::write(WriteStream & os) const
{
	os << "\\sqrt{" << cell(0) << '}';
}


void MathSqrtInset::normalize(NormalStream & os) const
{
	os << "[sqrt " << cell(0) << ']';
}

void MathSqrtInset::maple(MapleStream & os) const
{
	os << "sqrt(" << cell(0) << ')';
}

void MathSqrtInset::mathematica(MathematicaStream & os) const
{
	os << "Sqrt[" << cell(0) << ']';
}


void MathSqrtInset::octave(OctaveStream & os) const
{
	os << "sqrt(" << cell(0) << ')';
}


void MathSqrtInset::mathmlize(MathMLStream & os) const
{
	os << MTag("msqrt") << cell(0) << ETag("msqrt");
}
