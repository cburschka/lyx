/**
 * \file InsetMathBM.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Roider
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBM.h"

#include "MathStream.h"
#include "MathData.h"
#include "LaTeXFeatures.h"

#include <ostream>


namespace lyx {

InsetMathBM::InsetMathBM()
	: InsetMathNest(1)
{}


Inset * InsetMathBM::clone() const
{
	return new InsetMathBM(*this);
}


void InsetMathBM::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//FontSetChanger dummy(mi.base, "mathbf");
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	++dim.wid;  // for 'double stroke'
}


void InsetMathBM::draw(PainterInfo & pi, int x, int y) const
{
	//FontSetChanger dummy(pi.base, "mathbf");
	cell(0).draw(pi, x + 1, y);
	cell(0).draw(pi, x + 2, y);
	drawMarkers(pi, x, y);
}


void InsetMathBM::metricsT(TextMetricsInfo const & mi, Dimension & /*dim*/) const
{
	// FIXME: BROKEN!
	Dimension dim;
	cell(0).metricsT(mi, dim);
}


void InsetMathBM::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void InsetMathBM::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	features.require("bm");
}


void InsetMathBM::write(WriteStream & os) const
{
	os << "\\bm{" << cell(0) << "}";
}


void InsetMathBM::infoize(odocstream & os) const
{
	os << "bm ";
}


} // namespace lyx
