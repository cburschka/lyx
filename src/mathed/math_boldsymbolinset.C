/**
 * \file math_boldsymbolinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_boldsymbolinset.h"
#include "math_mathmlstream.h"
#include "math_data.h"
#include "LaTeXFeatures.h"
#include "support/std_ostream.h"

using std::auto_ptr;


MathBoldsymbolInset::MathBoldsymbolInset()
	: MathNestInset(1)
{}


auto_ptr<InsetBase> MathBoldsymbolInset::clone() const
{
	return auto_ptr<InsetBase>(new MathBoldsymbolInset(*this));
}


void MathBoldsymbolInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//FontSetChanger dummy(mi.base, "mathbf");
	cell(0).metrics(mi, dim);
	metricsMarkers(1);
	++dim.wid;  // for 'double stroke'
	dim_ = dim;
}


void MathBoldsymbolInset::draw(PainterInfo & pi, int x, int y) const
{
	//FontSetChanger dummy(pi.base, "mathbf");
	cell(0).draw(pi, x + 1, y);
	cell(0).draw(pi, x + 2, y);
	drawMarkers(pi, x, y);
}


void MathBoldsymbolInset::metricsT(TextMetricsInfo const & mi, Dimension & /*dim*/) const
{
	cell(0).metricsT(mi, dim_);
}


void MathBoldsymbolInset::drawT(TextPainter & pain, int x, int y) const
{
	cell(0).drawT(pain, x, y);
}


void MathBoldsymbolInset::validate(LaTeXFeatures & features) const
{
	MathNestInset::validate(features);
	features.require("amssymb");
}


void MathBoldsymbolInset::write(WriteStream & os) const
{
	os << "\\boldsymbol{" << cell(0) << "}";
}


void MathBoldsymbolInset::infoize(std::ostream & os) const
{
	os << "Boldsymbol ";
}
