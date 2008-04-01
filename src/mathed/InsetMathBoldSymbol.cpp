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

InsetMathBoldSymbol::InsetMathBoldSymbol(Kind kind)
	: InsetMathNest(1), kind_(kind)
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
	if (kind_ == AMS_BOLD)
		features.require("amsbsy");
	else
		features.require("bm");
}


void InsetMathBoldSymbol::write(WriteStream & os) const
{
	switch (kind_) {
	case AMS_BOLD:
		os << "\\boldsymbol{" << cell(0) << "}";
		break;
	case BM_BOLD:
		os << "\\bm{" << cell(0) << "}";
		break;
	case BM_HEAVY:
		os << "\\hm{" << cell(0) << "}";
		break;
	}
}


void InsetMathBoldSymbol::infoize(odocstream & os) const
{
	switch (kind_) {
	case AMS_BOLD:
		os << "Boldsymbol ";
		break;
	case BM_BOLD:
		os << "Boldsymbol (bm)";
		break;
	case BM_HEAVY:
		os << "Heavysymbol (bm)";
		break;
	}
}


} // namespace lyx
