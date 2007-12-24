/**
 * \file InsetMathBoldSymbol.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBoldSymbol.h"

#include "MathStream.h"
#include "MathData.h"
#include "LaTeXFeatures.h"

#include <ostream>


namespace lyx {

InsetMathBoldSymbol::InsetMathBoldSymbol()
	: InsetMathNest(1)
{}


Inset * InsetMathBoldSymbol::clone() const
{
	return new InsetMathBoldSymbol(*this);
}


void InsetMathBoldSymbol::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//FontSetChanger dummy(mi.base, "mathbf");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	++dim.wid;  // for 'double stroke'
}


void InsetMathBoldSymbol::draw(PainterInfo & pi, int x, int y) const
{
	//FontSetChanger dummy(pi.base, "mathbf");
	cell(0).draw(pi, x + 1, y);
	cell(0).draw(pi, x + 2, y);
	drawMarkers(pi, x, y);
}


void InsetMathBoldSymbol::metricsT(TextMetricsInfo const & mi, Dimension & /*dim*/) const
{
	// FIXME: BROKEN!
	Dimension dim;
	cell(0).metricsT(mi, dim);
}


void InsetMathBoldSymbol::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void InsetMathBoldSymbol::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	features.require("amssymb");
}


void InsetMathBoldSymbol::write(WriteStream & os) const
{
	os << "\\boldsymbol{" << cell(0) << "}";
}


void InsetMathBoldSymbol::infoize(odocstream & os) const
{
	os << "Boldsymbol ";
}


} // namespace lyx
