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
#include "math_support.h"

#include "LaTeXFeatures.h"
#include "LColor.h"

#include "support/std_ostream.h"

using std::auto_ptr;


namespace {

// color "none" (reset to default) needs special treatment
bool normalcolor(MathArray const & ar)
{
	return (asString(ar) == "none");
}

} // namespace anon


MathColorInset::MathColorInset(bool oldstyle)
	: MathNestInset(2), oldstyle_(oldstyle)
{}


auto_ptr<InsetBase> MathColorInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathColorInset(*this));
}


void MathColorInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(1).metrics(mi, dim);
	if (editing(mi.base.bv)) {
		FontSetChanger dummy(mi.base, "textnormal");
		cell(0).metrics(mi);
		dim  += cell(0).dim();
		w_ = mathed_char_width(mi.base.font, '[');
		dim.asc += 4;
		dim.des += 4;
		dim.wid += 2 * w_ + 4;
		metricsMarkers(dim);
	}
	dim_ = dim;
}


void MathColorInset::draw(PainterInfo & pi, int x, int y) const
{
	if (editing(pi.base.bv)) {
		FontSetChanger dummy(pi.base, "textnormal");
		drawMarkers(pi, x, y);
		drawStrBlack(pi, x, y, "[");
		x += w_;
		cell(0).draw(pi, x, y);
		x += cell(0).width();
		drawStrBlack(pi, x, y, "]");
		x += w_ + 2;
	}

	LColor_color origcol = pi.base.font.color();
	pi.base.font.setColor(lcolor.getFromGUIName(asString(cell(0))));
	cell(1).draw(pi, x, y);
	pi.base.font.setColor(origcol);
	setPosCache(pi, x, y);
}


void MathColorInset::validate(LaTeXFeatures & features) const
{
	MathNestInset::validate(features);
	if (!normalcolor(cell(0)))
		features.require("color");
}


void MathColorInset::write(WriteStream & os) const
{
	if (normalcolor(cell(0)))
		os << "{\\normalcolor " << cell(1) << '}';
	else if (oldstyle_)
		os << "{\\color" << '{' << cell(0) << '}' << cell(1) << '}';
	else
		os << "\\textcolor" << '{' << cell(0) << "}{" << cell(1) << '}';
}


void MathColorInset::normalize(NormalStream & os) const
{
	os << "[color " << cell(0) << ' ' << cell(1) << ']';
}


void MathColorInset::infoize(std::ostream & os) const
{
	os << "Color: " << cell(0);
}
