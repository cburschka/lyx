/**
 * \file InsetMathBinom.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathBinom.h"
#include "MathData.h"
#include "MathSupport.h"
#include "MathStream.h"


namespace lyx {

InsetMathBinom::InsetMathBinom(bool choose)
	: choose_(choose)
{}


Inset * InsetMathBinom::clone() const
{
	return new InsetMathBinom(*this);
}


int InsetMathBinom::dw(int height) const
{
	int w = height / 5;
	if (w > 15)
		w = 15;
	if (w < 6)
		w = 6;
	return w;
}


void InsetMathBinom::metrics(MetricsInfo & mi, Dimension & dim) const
{
	ScriptChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim.asc = cell(0).height() + 4 + 5;
	dim.des = cell(1).height() + 4 - 5;
	dim.wid = std::max(cell(0).width(), cell(1).width()) + 2 * dw(dim.height()) + 4;
	metricsMarkers2(dim);
	// Cache the inset dimension. 
	setDimCache(mi, dim);
}


void InsetMathBinom::draw(PainterInfo & pi, int x, int y) const
{
	Dimension const dim = dimension(*pi.base.bv);
	int m = x + dim.width() / 2;
	ScriptChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, y - cell(0).descent() - 3 - 5);
	cell(1).draw(pi, m - cell(1).width() / 2, y + cell(1).ascent()  + 3 - 5);
	mathed_draw_deco(pi, x, y - dim.ascent(), dw(dim.height()), dim.height(), from_ascii("("));
	mathed_draw_deco(pi, x + dim.width() - dw(dim.height()), y - dim.ascent(),
		dw(dim.height()), dim.height(), from_ascii(")"));
	drawMarkers2(pi, x, y);
}


bool InsetMathBinom::extraBraces() const
{
	return choose_;
}


void InsetMathBinom::write(WriteStream & os) const
{
	if (choose_)
		os << '{' << cell(0) << " \\choose " << cell(1) << '}';
	else
		os << "\\binom{" << cell(0) << "}{" << cell(1) << '}';
}


void InsetMathBinom::normalize(NormalStream & os) const
{
	os << "[binom " << cell(0) << ' ' << cell(1) << ']';
}


} // namespace lyx
