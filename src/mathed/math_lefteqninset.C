#ifdef __GNUG__
#pragma implementation
#endif

#include "math_lefteqninset.h"
#include "math_support.h"
#include "support/LOstream.h"


MathLefteqnInset::MathLefteqnInset()
	: MathNestInset(1)
{}


MathInset * MathLefteqnInset::clone() const
{
	return new MathLefteqnInset(*this);
}


void MathLefteqnInset::metrics(MathMetricsInfo & mi) const
{
	cell(0).metrics(mi);
	dim_.a = cell(0).ascent() + 2;
	dim_.d = cell(0).descent() + 2;
	dim_.w = 4;
	metricsMarkers2();
}


void MathLefteqnInset::draw(MathPainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x + 2, y);
	drawMarkers2(pi, x, y);
}


string MathLefteqnInset::name() const
{
	return "lefteqn";
}


void MathLefteqnInset::infoize(std::ostream & os) const
{
	os << "Lefteqn ";
}


