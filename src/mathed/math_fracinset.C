#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fracinset.h"
#include "mathed/support.h"
#include "Painter.h"
#include "support/LOstream.h"


MathFracInset::MathFracInset(string const & name)
	: MathInset(2, name)
{}


MathInset * MathFracInset::clone() const
{   
	return new MathFracInset(*this);
}


void MathFracInset::metrics(MathStyles st)
{
	size_    = smallerStyleFrac(st);
	xcell(0).metrics(size_);
	xcell(1).metrics(size_);
	width_   = std::max(xcell(0).width(), xcell(1).width()) + 4; 
	ascent_  = xcell(0).height() + 4 + 5;
	descent_ = xcell(1).height() + 4 - 5; 
}


void MathFracInset::draw(Painter & pain, int x, int y)
{
	xo(x);
	yo(y);
	int m = x + width() / 2;
	xcell(0).draw(pain, m - xcell(0).width() / 2, y - xcell(0).descent() - 3 - 5);
	xcell(1).draw(pain, m - xcell(1).width() / 2, y + xcell(1).ascent()  + 3 - 5);
	
	if (name() == "frac")
		pain.line(x + 2, y - 5, x + width() - 4, y - 5, LColor::mathline);
}


void MathFracInset::write(std::ostream & os, bool fragile) const
{
	os << '\\' << name() << '{';
	cell(0).write(os, fragile);
	os << "}{";
	cell(1).write(os, fragile);
	os << '}';
}


void MathFracInset::writeNormal(std::ostream & os) const
{
	os << '[' << name() << ' ';
	cell(0).writeNormal(os);
	os << " ";
	cell(1).writeNormal(os);
	os << "] ";
}


bool MathFracInset::idxRight(int &, int &) const
{
	return false;
}

bool MathFracInset::idxLeft(int &, int &) const
{
	return false;
}


bool MathFracInset::idxUp(int & idx, int &) const
{
	if (idx == 0)
		return false;
	idx = 0;
	return true;
}

bool MathFracInset::idxDown(int & idx, int &) const
{
	if (idx == 1)
		return false;
	idx = 1;
	return true;
}

bool MathFracInset::idxFirstUp(int & idx, int & pos) const
{
	idx = 0;
	pos = 0;
	return true;
}

bool MathFracInset::idxFirstDown(int & idx, int & pos) const
{
	idx = 1;
	pos = 0;
	return true;
}

bool MathFracInset::idxLastUp(int & idx, int & pos) const
{
	idx = 0;
	pos = cell(idx).size();
	return true;
}

bool MathFracInset::idxLastDown(int & idx, int & pos) const
{
	idx = 1;
	pos = cell(idx).size();
	return true;
}

