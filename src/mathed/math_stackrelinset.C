/**
 * \file math_stackrelinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_stackrelinset.h"
#include "math_data.h"
#include "math_mathmlstream.h"


using std::max;
using std::auto_ptr;


MathStackrelInset::MathStackrelInset()
{}


auto_ptr<InsetBase> MathStackrelInset::doClone() const
{
	return auto_ptr<InsetBase>(new MathStackrelInset(*this));
}


void MathStackrelInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(1).metrics(mi);
	FracChanger dummy(mi.base);
	cell(0).metrics(mi);
	dim.wid = max(cell(0).width(), cell(1).width()) + 4;
	dim.asc = cell(1).ascent() + cell(0).height() + 4;
	dim.des = cell(1).descent();
	metricsMarkers(dim);
	dim_ = dim;
}


void MathStackrelInset::draw(PainterInfo & pi, int x, int y) const
{
	int m  = x + dim_.width() / 2;
	int yo = y - cell(1).ascent() - cell(0).descent() - 1;
	cell(1).draw(pi, m - cell(1).width() / 2, y);
	FracChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, yo);
	drawMarkers(pi, x, y);
}


void MathStackrelInset::write(WriteStream & os) const
{
	os << "\\stackrel{" << cell(0) << "}{" << cell(1) << '}';
}


void MathStackrelInset::normalize(NormalStream & os) const
{
	os << "[stackrel " << cell(0) << ' ' << cell(1) << ']';
}
