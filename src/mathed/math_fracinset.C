#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fracinset.h"
#include "mathed/support.h"
#include "Painter.h"
#include "support/LOstream.h"


MathFracInset::MathFracInset()
{}


MathInset * MathFracInset::clone() const
{   
	return new MathFracInset(*this);
}


void MathFracInset::metrics(MathStyles st) const
{
	size_    = smallerStyleFrac(st);
	xcell(0).metrics(size_);
	xcell(1).metrics(size_);
	width_   = std::max(xcell(0).width(), xcell(1).width()) + 4; 
	ascent_  = xcell(0).height() + 4 + 5;
	descent_ = xcell(1).height() + 4 - 5; 
}


void MathFracInset::draw(Painter & pain, int x, int y) const
{
	xo(x);
	yo(y);
	int m = x + width() / 2;
	xcell(0).draw(pain, m - xcell(0).width() / 2, y - xcell(0).descent() - 3 - 5);
	xcell(1).draw(pain, m - xcell(1).width() / 2, y + xcell(1).ascent()  + 3 - 5);
	pain.line(x + 2, y - 5, x + width() - 4, y - 5, LColor::mathline);
}


void MathFracInset::write(std::ostream & os, bool fragile) const
{
	os << "\\frac{";
	cell(0).write(os, fragile);
	os << "}{";
	cell(1).write(os, fragile);
	os << '}';
}


void MathFracInset::writeNormal(std::ostream & os) const
{
	os << "[frac ";
	cell(0).writeNormal(os);
	os << " ";
	cell(1).writeNormal(os);
	os << "] ";
}
