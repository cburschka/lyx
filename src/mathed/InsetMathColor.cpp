/**
 * \file InsetMathColor.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Color.h"

#include "InsetMathColor.h"
#include "LaTeXFeatures.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "MetricsInfo.h"

#include <ostream>


namespace lyx {

InsetMathColor::InsetMathColor(bool oldstyle, ColorCode color)
	: InsetMathNest(1), oldstyle_(oldstyle),
	  color_(from_utf8(lcolor.getLaTeXName(color)))
{}


InsetMathColor::InsetMathColor(bool oldstyle, docstring const & color)
	: InsetMathNest(1), oldstyle_(oldstyle), color_(color)
{}


Inset * InsetMathColor::clone() const
{
	return new InsetMathColor(*this);
}


void InsetMathColor::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	metricsMarkers(dim);
}


void InsetMathColor::draw(PainterInfo & pi, int x, int y) const
{
	ColorCode origcol = pi.base.font.color();
	pi.base.font.setColor(lcolor.getFromLaTeXName(to_utf8(color_)));
	cell(0).draw(pi, x + 1, y);
	pi.base.font.setColor(origcol);
	drawMarkers(pi, x, y);
	setPosCache(pi, x, y);
}


/// color "none" (reset to default) needs special treatment
static bool normalcolor(docstring const & color)
{
	return color == "none";
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
		os << "{\\color{" << color_ << '}' << cell(0) << '}';
	else
		os << "\\textcolor{" << color_ << "}{" << cell(0) << '}';
}


void InsetMathColor::normalize(NormalStream & os) const
{
	os << "[color " << color_ << ' ' << cell(0) << ']';
}


void InsetMathColor::infoize(odocstream & os) const
{
	os << "Color: " << color_;
}


} // namespace lyx
