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
#include "support/LOstream.h"


MathBoldsymbolInset::MathBoldsymbolInset()
	: MathNestInset(1)
{}


MathInset * MathBoldsymbolInset::clone() const
{
	return new MathBoldsymbolInset(*this);
}


void MathBoldsymbolInset::metrics(MathMetricsInfo & mi) const
{
	//FontSetChanger dummy(mi.base, "mathbf");
	dim_ = cell(0).metrics(mi);
	metricsMarkers(1);
	++dim_.w;  // for 'double stroke'
}


void MathBoldsymbolInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//FontSetChanger dummy(pi.base, "mathbf");
	cell(0).draw(pi, x + 1, y);
	cell(0).draw(pi, x + 2, y);
	drawMarkers(pi, x, y);
}


void MathBoldsymbolInset::metricsT(TextMetricsInfo const & mi) const
{
	dim_ = cell(0).metricsT(mi);
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
