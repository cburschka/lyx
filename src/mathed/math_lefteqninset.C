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
#include "support/LOstream.h"

using std::auto_ptr;


MathLefteqnInset::MathLefteqnInset()
	: MathNestInset(1)
{}


auto_ptr<InsetBase> MathLefteqnInset::clone() const
{
	return auto_ptr<InsetBase>(new MathLefteqnInset(*this));
}


void MathLefteqnInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi);
	dim_.asc = cell(0).ascent() + 2;
	dim_.des = cell(0).descent() + 2;
	dim_.wid = 4;
	metricsMarkers();
	dim = dim_;
}


void MathLefteqnInset::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 2, y);
	drawMarkers(pi, x, y);
}


string MathLefteqnInset::name() const
{
	return "lefteqn";
}


void MathLefteqnInset::infoize(std::ostream & os) const
{
	os << "Lefteqn ";
}
