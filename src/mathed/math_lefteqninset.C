#ifdef __GNUG__
#pragma implementation
#endif

#include "math_lefteqninset.h"
#include "math_mathmlstream.h"
#include "math_support.h"


MathLefteqnInset::MathLefteqnInset()
	: MathNestInset(1)
{}


MathInset * MathLefteqnInset::clone() const
{
	return new MathLefteqnInset(*this);
}


void MathLefteqnInset::metrics(MathMetricsInfo const & mi) const
{
	MathNestInset::metrics(mi);
	ascent_  = xcell(0).ascent() + 2;
	descent_ = xcell(0).descent() + 2;
	width_   = 4;
}


void MathLefteqnInset::draw(Painter & pain, int x, int y) const
{
	xcell(0).draw(pain, x + 2, y);
	//mathed_draw_framebox(pain, x, y, this);
}


void MathLefteqnInset::write(WriteStream & os) const
{
	os << "\\lefteqn{" << cell(0) << '}';
}


void MathLefteqnInset::normalize(NormalStream & os) const
{
	os << "[lefteqn " << cell(0) << ']';
}
