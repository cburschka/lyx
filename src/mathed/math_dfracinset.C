/**
 * \file math_dfracinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_dfracinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "LaTeXFeatures.h"
#include "LColor.h"
#include "frontends/Painter.h"


using std::string;
using std::max;
using std::auto_ptr;


MathDfracInset::MathDfracInset()
	: MathFracInset(false)
{}


auto_ptr<InsetBase> MathDfracInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathDfracInset(*this));
}


void MathDfracInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.wid = max(cell(0).width(), cell(1).width()) + 2;
	dim_.asc = cell(0).height() + 2 + 5;
	dim_.des = cell(1).height() + 2 - 5;
	dim = dim_;
}


void MathDfracInset::draw(PainterInfo & pi, int x, int y) const
{
	int m = x + dim_.wid / 2;
	cell(0).draw(pi, m - cell(0).width() / 2, y - cell(0).descent() - 2 - 5);
	cell(1).draw(pi, m - cell(1).width() / 2, y + cell(1).ascent()  + 2 - 5);
	pi.pain.line(x + 1, y - 5, x + dim_.wid - 2, y - 5, LColor::math);
	setPosCache(pi, x, y);
}


string MathDfracInset::name() const
{
	return "dfrac";
}


void MathDfracInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mdfrac") << cell(0) << cell(1) << ETag("mdfrac");
}


void MathDfracInset::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	MathNestInset::validate(features);
}
