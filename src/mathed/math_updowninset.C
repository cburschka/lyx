#ifdef __GNUG__
#pragma implementation
#endif

#include "math_updowninset.h"
#include "support/LOstream.h"


MathUpDownInset::MathUpDownInset()
	: MathInset(2), up_(false), down_(false)
{}

MathUpDownInset::MathUpDownInset(bool up, bool down)
	: MathInset(2), up_(up), down_(down)
{}


MathInset * MathUpDownInset::clone() const
{   
	return new MathUpDownInset(*this);
}


bool MathUpDownInset::up() const
{
	return up_;
}

bool MathUpDownInset::down() const
{
	return down_;
}

void MathUpDownInset::up(bool b)
{
	up_ = b;
}

void MathUpDownInset::down(bool b)
{
	down_ = b;
}


bool MathUpDownInset::idxRight(int &, int &) const
{
	return false;
}

bool MathUpDownInset::idxLeft(int &, int &) const
{
	return false;
}


bool MathUpDownInset::idxUp(int & idx, int & pos) const
{
	if (idx == 0 || !up()) 
		return false;
	idx = 0;
	pos = 0;
	return true;
}

bool MathUpDownInset::idxDown(int & idx, int & pos) const
{
	if (idx == 1 || !down()) 
		return false;
	idx = 1;
	pos = 0;
	return true;
}

bool MathUpDownInset::idxFirst(int & idx, int & pos) const
{
	idx = up() ? 0 : 1;
	pos = 0;
	return true;
}

bool MathUpDownInset::idxLast(int & idx, int & pos) const
{
	idx = down() ? 1 : 0;
	pos = cell(idx).size();
	return true;
}


bool MathUpDownInset::idxFirstUp(int & idx, int & pos) const
{
	if (!up()) 
		return false;
	idx = 0;
	pos = 0;
	return true;
}

bool MathUpDownInset::idxFirstDown(int & idx, int & pos) const
{
	if (!down()) 
		return false;
	idx = 1;
	pos = 0;
	return true;
}

bool MathUpDownInset::idxLastUp(int & idx, int & pos) const
{
	if (!up()) 
		return false;
	idx = 0;
	pos = cell(idx).size();
	return true;
}

bool MathUpDownInset::idxLastDown(int & idx, int & pos) const
{
	if (!down()) 
		return false;
	idx = 1;
	pos = cell(idx).size();
	return true;
}


void MathUpDownInset::idxDelete(int & idx, bool & popit, bool & deleteit)
{
	if (idx == 0) 
		up(false);
	else
		down(false);
	popit = true;
	deleteit = !(up() || down());
}

void MathUpDownInset::Write(std::ostream & os, bool fragile) const
{
	if (up()) {
		os << "^{";
		cell(0).Write(os, fragile);
		os << "}";
	}
	if (down()) {
		os << "_{";
		cell(1).Write(os, fragile);
		os << "}";
	}
}

void MathUpDownInset::Metrics(MathStyles st, int asc, int des)
{
	size_ = st;
	MathStyles tt = smallerStyleScript(st);
	
	if (up())
		xcell(0).Metrics(tt);
	if (down())
		xcell(1).Metrics(tt);

	// we assume that asc, des, wid are the metrics of the item in front
	// of this MathScriptInset
	width_   = std::max(xcell(0).width(), xcell(1).width());
	ascent_  = up()   ? xcell(0).height() + asc : 0;
	descent_ = down() ? xcell(1).height() + des : 0;
	dy0_     = - asc  - xcell(0).descent();
	dy1_     =   des + xcell(1).ascent();
}


void MathUpDownInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(y);
	if (up())
		xcell(0).draw(pain, x, y + dy0_);
	if (down())
		xcell(1).draw(pain, x, y + dy1_);
}

