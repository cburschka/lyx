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

#include "InsetMathDFrac.h"
#include "MathData.h"
#include "MathStream.h"
#include "LaTeXFeatures.h"
#include "frontends/Painter.h"


namespace lyx {

InsetMathDFrac::InsetMathDFrac()
	: InsetMathFrac()
{}


Inset * InsetMathDFrac::clone() const
{
	return new InsetMathDFrac(*this);
}


void InsetMathDFrac::metrics(MetricsInfo & mi, Dimension & dim) const
{
	Dimension dim0, dim1;
	cell(0).metrics(mi, dim0);
	cell(1).metrics(mi, dim1);
	dim.wid = std::max(dim0.wid, dim1.wid) + 2;
	dim.asc = dim0.height() + 2 + 5;
	dim.des = dim1.height() + 2 - 5;
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathDFrac::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	Dimension const & dim0 = cell(0).dimension(*pi.base.bv);
	Dimension const & dim1 = cell(1).dimension(*pi.base.bv);
	int m = x + dim.wid / 2;
	cell(0).draw(pi, m - dim0.wid / 2, y - dim0.des - 2 - 5);
	cell(1).draw(pi, m - dim1.wid / 2, y + dim1.asc  + 2 - 5);
	pi.pain.line(x + 1, y - 5, x + dim.wid - 2, y - 5, Color_math);
	setPosCache(pi, x, y);
}


docstring InsetMathDFrac::name() const
{
	return from_ascii("dfrac");
}


void InsetMathDFrac::mathmlize(MathStream & os) const
{
	os << MTag("mdfrac") << cell(0) << cell(1) << ETag("mdfrac");
}


void InsetMathDFrac::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathNest::validate(features);
}


} // namespace lyx
