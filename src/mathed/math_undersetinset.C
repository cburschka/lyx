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
	cell(1).metrics(mi);
	MathFracChanger dummy(mi.base);
	cell(0).metrics(mi);
	dim_.w = max(cell(0).width(), cell(1).width()) + 4;
	dim_.a = cell(1).ascent();
	dim_.d = cell(1).descent() + cell(0).height() + 4;
}


void MathUndersetInset::draw(MathPainterInfo & pi, int x, int y) const
{
	int m  = x + width() / 2;
	int yo = y + cell(1).descent() + cell(0).ascent() + 1;
	cell(1).draw(pi, m - cell(1).width() / 2, y);
	MathFracChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, yo);
}


void MathUndersetInset::write(WriteStream & os) const
{
	os << "\\underset{" << cell(0) << "}{" << cell(1) << '}';
}


void MathUndersetInset::normalize(NormalStream & os) const
{
	os << "[underset " << cell(0) << ' ' << cell(1) << ']';
}
