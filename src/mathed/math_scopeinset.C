#ifdef __GNUG__
#pragma implementation
#endif

#include "math_scopeinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support.h"
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
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent();
	width_   = xcell(0).width() + mathed_string_width(LM_TC_TEX, st, "{}");
}


void MathScopeInset::draw(Painter & pain, int x, int y) const
{ 
	xo(x);
	yo(y);
	int d = mathed_char_width(LM_TC_TEX, size_, '{');
	drawChar(pain, LM_TC_TEX, size_, x, y, '{');
	xcell(0).draw(pain, x + d, y); 
	drawChar(pain, LM_TC_TEX, size_, x + width_ - d, y, '}');
	//pain.rectangle(x, y - ascent_, width_, height(), LColor::mathline);
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
