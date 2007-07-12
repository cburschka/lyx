/**
 * \file InsetMathOverset.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathOverset.h"
#include "MathData.h"
#include "MathStream.h"

#include "Cursor.h"
#include "LaTeXFeatures.h"


namespace lyx {

using std::max;
using std::auto_ptr;


auto_ptr<Inset> InsetMathOverset::doClone() const
{
	return auto_ptr<Inset>(new InsetMathOverset(*this));
}


bool InsetMathOverset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(1).metrics(mi);
	FracChanger dummy(mi.base);
	cell(0).metrics(mi);
	dim.wid = max(cell(0).width(), cell(1).width()) + 4;
	dim.asc = cell(1).ascent() + cell(0).height() + 4;
	dim.des = cell(1).descent();
	metricsMarkers(dim);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathOverset::draw(PainterInfo & pi, int x, int y) const
{
	int m  = x + width() / 2;
	int yo = y - cell(1).ascent() - cell(0).descent() - 1;
	cell(1).draw(pi, m - cell(1).width() / 2, y);
	FracChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, yo);
	drawMarkers(pi, x, y);
}


bool InsetMathOverset::idxFirst(Cursor & cur) const
{
	cur.idx() = 1;
	cur.pos() = 0;
	return true;
}


bool InsetMathOverset::idxLast(Cursor & cur) const
{
	cur.idx() = 1;
	cur.pos() = cur.lastpos();
	return true;
}


void InsetMathOverset::write(WriteStream & os) const
{
	os << "\\overset{" << cell(0) << "}{" << cell(1) << '}';
}


void InsetMathOverset::normalize(NormalStream & os) const
{
	os << "[overset " << cell(0) << ' ' << cell(1) << ']';
}


void InsetMathOverset::validate(LaTeXFeatures & features) const
{
	features.require("amsmath");
	InsetMathNest::validate(features);
}


} // namespace lyx
