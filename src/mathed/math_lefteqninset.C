#ifdef __GNUG__
#pragma implementation
#endif

#include "math_lefteqninset.h"
#include "math_support.h"


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
}


void MathLefteqnInset::draw(MathPainterInfo & pain, int x, int y) const
{
	cell(0).draw(pain, x + 2, y);
	//mathed_draw_framebox(pain, x, y, this);
}


string MathLefteqnInset::name() const
{
	return "lefteqn";
}
