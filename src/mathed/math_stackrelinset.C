#ifdef __GNUG__
#pragma implementation
#endif

#include "math_stackrelinset.h"
#include "mathed/support.h"
#include "support/LOstream.h"


MathStackrelInset::MathStackrelInset()
{}


MathInset * MathStackrelInset::clone() const
{   
	return new MathStackrelInset(*this);
}


void MathStackrelInset::metrics(MathMetricsInfo const & mi) const
{
	MathMetricsInfo m = mi;
	smallerStyleFrac(m);
	xcell(0).metrics(m);
	xcell(1).metrics(mi);
	width_   = std::max(xcell(0).width(), xcell(1).width()) + 4; 
	ascent_  = xcell(1).ascent() + xcell(0).height() + 4;
	descent_ = xcell(1).descent();
}


void MathStackrelInset::draw(Painter & pain, int x, int y) const
{
	int m = x + width() / 2;
	xcell(0).draw(pain, m - xcell(0).width() / 2, y - xcell(0).height() - 4);
	xcell(1).draw(pain, m - xcell(1).width() / 2, y);
}


void MathStackrelInset::write(MathWriteInfo & os) const
{
	os << "\\stackrel{" << cell(0) << "}{" << cell(1) << '}';
}


void MathStackrelInset::writeNormal(std::ostream & os) const
{
	os << "[stackrel ";
	cell(0).writeNormal(os);
	os << " ";
	cell(1).writeNormal(os);
	os << "] ";
}
