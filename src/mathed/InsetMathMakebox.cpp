/**
 * \file InsetMathMakebox.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Ling Li
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathMakebox.h"
#include "MathData.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "support/std_ostream.h"


namespace lyx {

using std::auto_ptr;


InsetMathMakebox::InsetMathMakebox()
	: InsetMathNest(3)
{}


auto_ptr<Inset> InsetMathMakebox::doClone() const
{
	return auto_ptr<Inset>(new InsetMathMakebox(*this));
}


bool InsetMathMakebox::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy(mi.base, from_ascii("textnormal"));

	Dimension wdim;
	static docstring bracket = from_ascii("[");
	mathed_string_dim(mi.base.font, bracket, wdim);
	int w = wdim.wid;
	
	Dimension dim0;
	Dimension dim1;
	Dimension dim2;
	cell(0).metrics(mi, dim0);
	cell(1).metrics(mi, dim1);
	cell(2).metrics(mi, dim2);
	
	dim.wid = 5 + w + dim0.wid + w + 4 + w + dim1.wid + w + 4 + dim2.wid + 5;
	dim.asc = std::max(std::max(wdim.asc, dim0.asc), std::max(dim1.asc, dim2.asc));
	dim.des = std::max(std::max(wdim.des, dim0.des), std::max(dim1.des, dim2.des));
	dim.asc += 3;
	dim.des += 3;

	metricsMarkers(dim);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathMakebox::draw(PainterInfo & pi, int x, int y) const
{
	drawMarkers(pi, x, y);

	FontSetChanger dummy(pi.base, from_ascii("textnormal"));
	int w = mathed_char_width(pi.base.font, '[');

	x += 5;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w;
	cell(0).draw(pi, x, y);
	x += cell(0).width();
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w + 2;

	drawStrBlack(pi, x, y, from_ascii("["));
	x += w;
	cell(1).draw(pi, x, y);
	x += cell(1).width();
	drawStrBlack(pi, x, y, from_ascii("]"));
	x += w + 4;

	cell(2).draw(pi, x, y);
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


void InsetMathMakebox::infoize(odocstream & os) const
{
	os << "Makebox (width: " << cell(0)
	    << " pos: " << cell(1) << ")";
}


} // namespace lyx
