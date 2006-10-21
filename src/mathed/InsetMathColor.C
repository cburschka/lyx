/**
 * \file InsetMathColor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathColor.h"
#include "MathData.h"
#include "MathMLStream.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "LaTeXFeatures.h"

#include "support/std_ostream.h"


namespace lyx {

using std::auto_ptr;
using std::string;


namespace {

/// color "none" (reset to default) needs special treatment
bool normalcolor(string const & color)
{
	return color == "none";
}

} // namespace anon


InsetMathColor::InsetMathColor(bool oldstyle, LColor_color const & color)
	: InsetMathNest(1), oldstyle_(oldstyle),
	  color_(lcolor.getLaTeXName(color))
{}


InsetMathColor::InsetMathColor(bool oldstyle, string const & color)
	: InsetMathNest(1), oldstyle_(oldstyle), color_(color)
{}


auto_ptr<InsetBase> InsetMathColor::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathColor(*this));
}


void InsetMathColor::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
	dim_ = dim;
}


void InsetMathColor::draw(PainterInfo & pi, int x, int y) const
{
	LColor_color origcol = pi.base.font.color();
	pi.base.font.setColor(lcolor.getFromLaTeXName(color_));
	cell(0).draw(pi, x + 1, y);
	pi.base.font.setColor(origcol);
	drawMarkers(pi, x, y);
	setPosCache(pi, x, y);
}


void InsetMathColor::validate(LaTeXFeatures & features) const
{
	InsetMathNest::validate(features);
	if (!normalcolor(color_))
		features.require("color");
}


void InsetMathColor::write(WriteStream & os) const
{
	if (normalcolor(color_))
		// reset to default color inside another color inset
		os << "{\\normalcolor " << cell(0) << '}';
	else if (oldstyle_)
		os << "{\\color" << '{' << color_ << '}' << cell(0) << '}';
	else
		os << "\\textcolor" << '{' << color_ << "}{" << cell(0) << '}';
}


void InsetMathColor::normalize(NormalStream & os) const
{
	os << "[color " << color_ << ' ' << cell(0) << ']';
}


void InsetMathColor::infoize(std::ostream & os) const
{
	os << "Color: " << color_;
}


} // namespace lyx
