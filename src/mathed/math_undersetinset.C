#ifdef __GNUG__
#pragma implementation
#endif

#include "math_undersetinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"


MathUndersetInset::MathUndersetInset()
{}


MathInset * MathUndersetInset::clone() const
{   
	return new MathUndersetInset(*this);
}


void MathUndersetInset::metrics(MathMetricsInfo const & mi) const
{
	MathMetricsInfo m = mi;
	smallerStyleFrac(m);
	xcell(0).metrics(m);
	xcell(1).metrics(mi);
	width_   = std::max(xcell(0).width(), xcell(1).width()) + 4; 
	ascent_  = xcell(1).ascent();
	descent_ = xcell(1).descent() + xcell(0).height() + 4;
}


void MathUndersetInset::draw(Painter & pain, int x, int y) const
{
	int m  = x + width() / 2;
	int yo = y + xcell(1).descent() + xcell(0).ascent() + 1;
	xcell(0).draw(pain, m - xcell(0).width() / 2, yo);
	xcell(1).draw(pain, m - xcell(1).width() / 2, y);
}


void MathUndersetInset::write(WriteStream & os) const
{
	os << "\\underset{" << cell(0) << "}{" << cell(1) << '}';
}


void MathUndersetInset::normalize(NormalStream & os) const
{
	os << "[underset " << cell(0) << ' ' << cell(1) << ']';
}
