#ifdef __GNUG__
#pragma implementation
#endif

#include "math_undersetinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"


using std::max;


MathUndersetInset::MathUndersetInset()
{}


MathInset * MathUndersetInset::clone() const
{
	return new MathUndersetInset(*this);
}


void MathUndersetInset::metrics(MathMetricsInfo & mi) const
{
	xcell(1).metrics(mi);
	MathFracChanger dummy(mi.base);
	xcell(0).metrics(mi);
	width_   = max(xcell(0).width(), xcell(1).width()) + 4;
	ascent_  = xcell(1).ascent();
	descent_ = xcell(1).descent() + xcell(0).height() + 4;
}


void MathUndersetInset::draw(MathPainterInfo & pi, int x, int y) const
{
	int m  = x + width() / 2;
	int yo = y + xcell(1).descent() + xcell(0).ascent() + 1;
	xcell(1).draw(pi, m - xcell(1).width() / 2, y);
	MathFracChanger dummy(pi.base);
	xcell(0).draw(pi, m - xcell(0).width() / 2, yo);
}


void MathUndersetInset::write(WriteStream & os) const
{
	os << "\\underset{" << cell(0) << "}{" << cell(1) << '}';
}


void MathUndersetInset::normalize(NormalStream & os) const
{
	os << "[underset " << cell(0) << ' ' << cell(1) << ']';
}
