#include "math_undersetinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"


using std::max;
using std::auto_ptr;


MathUndersetInset::MathUndersetInset()
{}


auto_ptr<InsetBase> MathUndersetInset::clone() const
{
	return auto_ptr<InsetBase>(new MathUndersetInset(*this));
}


void MathUndersetInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(1).metrics(mi);
	FracChanger dummy(mi.base);
	cell(0).metrics(mi);
	dim_.wid = max(cell(0).width(), cell(1).width()) + 4;
	dim_.asc = cell(1).ascent();
	dim_.des = cell(1).descent() + cell(0).height() + 4;
	dim = dim_;
}


void MathUndersetInset::draw(PainterInfo & pi, int x, int y) const
{
	int m  = x + pi.width / 2;
	int yo = y + cell(1).descent() + cell(0).ascent() + 1;
	cell(1).draw(pi, m - cell(1).width() / 2, y);
	FracChanger dummy(pi.base);
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
