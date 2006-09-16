/**
 * \file math_makeboxinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ling Li
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_makeboxinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"
#include "math_support.h"

#include "support/std_ostream.h"

using std::auto_ptr;


InsetMathMakebox::InsetMathMakebox()
	: InsetMathNest(3)
{}


auto_ptr<InsetBase> InsetMathMakebox::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathMakebox(*this));
}


void InsetMathMakebox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, "textnormal");
	w_ = mathed_char_width(mi.base.font, '[');
	InsetMathNest::metrics(mi);
	dim   = cell(0).dim();
	dim  += cell(1).dim();
	dim  += cell(2).dim();
	dim.wid += 4 * w_ + 4;
	metricsMarkers(dim);
	dim_ = dim;
}


void InsetMathMakebox::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	drawMarkers(pi, x, y);

	drawStrBlack(pi, x, y, "[");
	x += w_;
	cell(0).draw(pi, x, y);
	x += cell(0).width();
	drawStrBlack(pi, x, y, "]");
	x += w_ + 2;

	drawStrBlack(pi, x, y, "[");
	x += w_;
	cell(1).draw(pi, x, y);
	x += cell(1).width();
	drawStrBlack(pi, x, y, "]");
	x += w_ + 2;

	cell(2).draw(pi, x, y);
	setPosCache(pi, x, y);
}


void InsetMathMakebox::write(WriteStream & os) const
{
	os << "\\makebox";
	os << '[' << cell(0) << ']';
	if (cell(1).size())
		os << '[' << cell(1) << ']';
	os << '{' << cell(2) << '}';
}


void InsetMathMakebox::normalize(NormalStream & os) const
{
	os << "[makebox " << cell(0) << ' ' << cell(1) << ' ' << cell(2) << ']';
}


void InsetMathMakebox::infoize(std::ostream & os) const
{
	os << "Makebox (width: " << cell(0)
	    << " pos: " << cell(1) << ")";
}
