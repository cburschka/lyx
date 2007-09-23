/**
 * \file InsetMathDFrac.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathTFrac.h"

#include "MathData.h"
#include "MathStream.h"

#include "LaTeXFeatures.h"
#include "Color.h"

#include "frontends/Painter.h"


namespace lyx {

InsetMathTFrac::InsetMathTFrac()
	: InsetMathFrac()
{}


Inset * InsetMathTFrac::clone() const
{
	return new InsetMathTFrac(*this);
}


void InsetMathTFrac::metrics(MetricsInfo & mi, Dimension & dim) const
{
	StyleChanger dummy(mi.base, LM_ST_SCRIPT);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim.wid = std::max(cell(0).width(), cell(1).width()) + 2;
	dim.asc = cell(0).height() + 2 + 5;
	dim.des = cell(1).height() + 2 - 5;
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathTFrac::draw(PainterInfo & pi, int x, int y) const
{
	StyleChanger dummy(pi.base, LM_ST_SCRIPT);
	Dimension const dim = dimension(*pi.base.bv);
	int m = x + dim.wid / 2;
	cell(0).draw(pi, m - cell(0).width() / 2, y - cell(0).descent() - 2 - 5);
	cell(1).draw(pi, m - cell(1).width() / 2, y + cell(1).ascent()  + 2 - 5);
	pi.pain.line(x + 1, y - 5, x + dim.wid - 2, y - 5, Color::math);
	setPosCache(pi, x, y);
}


docstring InsetMathTFrac::name() const
{
	return from_ascii("tfrac");
}


void InsetMathTFrac::mathmlize(MathStream & os) const
{
	os << MTag("mtfrac") << cell(0) << cell(1) << ETag("mtfrac");
}


void InsetMathTFrac::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathNest::validate(features);
}


} // namespace lyx
