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

#include "MathStream.h"
#include "MathData.h"

#include <ostream>


namespace lyx {

InsetMathEnsureMath::InsetMathEnsureMath()
	: InsetMathNest(1)
{}


Inset * InsetMathEnsureMath::clone() const
{
	return new InsetMathEnsureMath(*this);
}


void InsetMathEnsureMath::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "mathnormal");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathEnsureMath::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "mathnormal");
	cell(0).draw(pi, x, y);
	drawMarkers(pi, x, y);
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


void InsetMathEnsureMath::infoize(odocstream & os) const
{
	os << "EnsureMath";
}


} // namespace lyx
