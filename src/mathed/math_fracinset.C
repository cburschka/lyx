#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <functional>

#include "math_fracinset.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"


MathFracInset::MathFracInset(MathInsetTypes ot)
	: MathInset("frac", ot, 2)
{
	if (objtype == LM_OT_STACKREL) 
		SetName("stackrel");
}


MathInset * MathFracInset::clone() const
{   
	return new MathFracInset(*this);
}


void MathFracInset::Metrics(MathStyles st)
{
	xcell(0).Metrics(st);
	xcell(1).Metrics(st);
	size_    = st;
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
	
	if (objtype == LM_OT_FRAC)
		pain.line(x + 2, y - 5, x + width() - 4, y - 5, LColor::mathline);
}


void MathFracInset::Write(std::ostream & os, bool fragile) const
{
	os << '\\' << name() << '{';
	cell(0).Write(os, fragile);
	os << "}{";
	cell(1).Write(os, fragile);
	os << '}';
}


void MathFracInset::WriteNormal(std::ostream & os) const
{
	os << '[' << name() << ' ';
	cell(0).WriteNormal(os);
	os << " ";
	cell(1).WriteNormal(os);
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

