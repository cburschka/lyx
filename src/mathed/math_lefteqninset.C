/**
 * \file math_lefteqninset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_lefteqninset.h"
#include "math_data.h"
#include "support/std_ostream.h"


using std::string;
using std::auto_ptr;


InsetMathLefteqn::InsetMathLefteqn()
	: InsetMathNest(1)
{}


auto_ptr<InsetBase> InsetMathLefteqn::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathLefteqn(*this));
}


void InsetMathLefteqn::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
	dim.asc += 2;
	dim.des += 2;
	dim.wid = 4;
	metricsMarkers(dim);
	dim_ = dim;
}


void InsetMathLefteqn::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 2, y);
	drawMarkers(pi, x, y);
}


string InsetMathLefteqn::name() const
{
	return "lefteqn";
}


void InsetMathLefteqn::infoize(std::ostream & os) const
{
	os << "Lefteqn ";
}
