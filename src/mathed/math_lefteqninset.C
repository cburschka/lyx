
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


Dimension MathLefteqnInset::metrics(MetricsInfo & mi) const
{
	cell(0).metrics(mi);
	dim_.asc = cell(0).ascent() + 2;
	dim_.des = cell(0).descent() + 2;
	dim_.wid = 4;
	metricsMarkers();
	return dim_;
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
