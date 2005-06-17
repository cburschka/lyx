/**
 * \file math_colorinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_colorinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"

#include "LaTeXFeatures.h"

#include "support/std_ostream.h"

using std::auto_ptr;
using std::string;


namespace {

/// color "none" (reset to default) needs special treatment
bool normalcolor(string const & color)
{
	return color == "none";
}

} // namespace anon


MathColorInset::MathColorInset(bool oldstyle, LColor_color const & color)
	: MathNestInset(1), oldstyle_(oldstyle),
	  color_(lcolor.getLaTeXName(color))
{}


MathColorInset::MathColorInset(bool oldstyle, string const & color)
	: MathNestInset(1), oldstyle_(oldstyle), color_(color)
{}


auto_ptr<InsetBase> MathColorInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathColorInset(*this));
}


void MathColorInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	dim_ = dim;
}


void MathColorInset::draw(PainterInfo & pi, int x, int y) const
{
	LColor_color origcol = pi.base.font.color();
	pi.base.font.setColor(lcolor.getFromLaTeXName(color_));
	cell(0).draw(pi, x + 1, y);
	pi.base.font.setColor(origcol);
	drawMarkers(pi, x, y);
	setPosCache(pi, x, y);
}


void MathColorInset::validate(LaTeXFeatures & features) const
{
	MathNestInset::validate(features);
	if (!normalcolor(color_))
		features.require("color");
}


void MathColorInset::write(WriteStream & os) const
{
	if (normalcolor(color_))
		// reset to default color inside another color inset
		os << "{\\normalcolor " << cell(0) << '}';
	else if (oldstyle_)
		os << "{\\color" << '{' << color_ << '}' << cell(0) << '}';
	else
		os << "\\textcolor" << '{' << color_ << "}{" << cell(0) << '}';
}


void MathColorInset::normalize(NormalStream & os) const
{
	os << "[color " << color_ << ' ' << cell(0) << ']';
}


void MathColorInset::infoize(std::ostream & os) const
{
	os << "Color: " << color_;
}
