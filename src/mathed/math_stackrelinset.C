#ifdef __GNUG__
#pragma implementation
#endif

#include "math_stackrelinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"


using std::max;


MathStackrelInset::MathStackrelInset()
{}


MathInset * MathStackrelInset::clone() const
{
	return new MathStackrelInset(*this);
}


void MathStackrelInset::metrics(MathMetricsInfo & mi) const
{
	cell(1).metrics(mi);
	MathFracChanger dummy(mi.base);
	cell(0).metrics(mi);
	dim_.w = max(cell(0).width(), cell(1).width()) + 4;
	dim_.a = cell(1).ascent() + cell(0).height() + 4;
	dim_.d = cell(1).descent();
}


void MathStackrelInset::draw(MathPainterInfo & pi, int x, int y) const
{
	int m  = x + width() / 2;
	int yo = y - cell(1).ascent() - cell(0).descent() - 1;
	cell(1).draw(pi, m - cell(1).width() / 2, y);
	MathFracChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, yo);
}


void MathStackrelInset::write(WriteStream & os) const
{
	os << "\\stackrel{" << cell(0) << "}{" << cell(1) << '}';
}


void MathStackrelInset::normalize(NormalStream & os) const
{
	os << "[stackrel " << cell(0) << ' ' << cell(1) << ']';
}
