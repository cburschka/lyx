#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scopeinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support/LOstream.h"


MathScopeInset::MathScopeInset()
	: MathNestInset(1)
{}


MathInset * MathScopeInset::clone() const
{   
	return new MathScopeInset(*this);
}


void MathScopeInset::metrics(MathStyles st) const
{
	xcell(0).metrics(st);
	size_    = st;
	ascent_  = xcell(0).ascent()  + 2;
	descent_ = xcell(0).descent() + 2;
	width_   = xcell(0).width()   + 4;
}


void MathScopeInset::draw(Painter & pain, int x, int y) const
{ 
	xo(x);
	yo(y);
	xcell(0).draw(pain, x + 2, y); 
	pain.rectangle(x, y - ascent_, width_, height(), LColor::mathline);
}


void MathScopeInset::write(std::ostream & os, bool fragile) const
{
	os << '{';
	cell(0).write(os, fragile); 
	os << '}';
}


void MathScopeInset::writeNormal(std::ostream & os) const
{
	os << "[scope ";
	cell(0).writeNormal(os); 
	os << "] ";
}
