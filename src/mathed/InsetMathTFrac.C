/**
 * \file InsetMathDFrac.C
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
#include "MathMLStream.h"

#include "LaTeXFeatures.h"
#include "LColor.h"

#include "frontends/Painter.h"


namespace lyx {


using std::string;
using std::max;
using std::auto_ptr;


InsetMathTFrac::InsetMathTFrac()
	: InsetMathFrac()
{}


auto_ptr<InsetBase> InsetMathTFrac::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathTFrac(*this));
}


void InsetMathTFrac::metrics(MetricsInfo & mi, Dimension & dim) const
{
	StyleChanger dummy(mi.base, LM_ST_SCRIPT);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.wid = max(cell(0).width(), cell(1).width()) + 2;
	dim_.asc = cell(0).height() + 2 + 5;
	dim_.des = cell(1).height() + 2 - 5;
	dim = dim_;
}


void InsetMathTFrac::draw(PainterInfo & pi, int x, int y) const
{
	StyleChanger dummy(pi.base, LM_ST_SCRIPT);
	int m = x + dim_.wid / 2;
	cell(0).draw(pi, m - cell(0).width() / 2, y - cell(0).descent() - 2 - 5);
	cell(1).draw(pi, m - cell(1).width() / 2, y + cell(1).ascent()  + 2 - 5);
	pi.pain.line(x + 1, y - 5, x + dim_.wid - 2, y - 5, LColor::math);
	setPosCache(pi, x, y);
}


string InsetMathTFrac::name() const
{
	return "tfrac";
}


void InsetMathTFrac::mathmlize(MathMLStream & os) const
{
	os << MTag("mtfrac") << cell(0) << cell(1) << ETag("mtfrac");
}


void InsetMathTFrac::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathNest::validate(features);
}


} // namespace lyx
