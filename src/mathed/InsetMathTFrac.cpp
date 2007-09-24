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
	Dimension dim0;
	cell(0).metrics(mi, dim0);
	Dimension dim1;
	cell(1).metrics(mi, dim1);
	dim.wid = std::max(dim0.width(), dim1.width()) + 2;
	dim.asc = dim0.height() + 2 + 5;
	dim.des = dim1.height() + 2 - 5;
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathTFrac::draw(PainterInfo & pi, int x, int y) const
{
	StyleChanger dummy(pi.base, LM_ST_SCRIPT);
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	int m = x + dim.wid / 2;
	cell(0).draw(pi, m - dim0.width() / 2, y - dim0.descent() - 2 - 5);
	cell(1).draw(pi, m - dim1.width() / 2, y + dim1.ascent()  + 2 - 5);
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
