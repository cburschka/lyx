#ifdef __GNUG__
#pragma implementation
#endif

#include "math_specialcharinset.h"
#include "support/LOstream.h"
#include "support.h"


MathSpecialCharInset::MathSpecialCharInset(char c)
	: char_(c)
{}


MathInset * MathSpecialCharInset::clone() const
{   
	return new MathSpecialCharInset(*this);
}


int MathSpecialCharInset::ascent() const
{
	return mathed_char_ascent(LM_TC_CONST, mi_, char_);
}


int MathSpecialCharInset::descent() const
{
	return mathed_char_descent(LM_TC_CONST, mi_, char_);
}


int MathSpecialCharInset::width() const
{
	return mathed_char_width(LM_TC_CONST, mi_, char_);
}


void MathSpecialCharInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
}


void MathSpecialCharInset::draw(Painter & pain, int x, int y) const
{ 
	drawChar(pain, LM_TC_CONST, mi_, x, y, char_);
}


void MathSpecialCharInset::write(MathWriteInfo & os) const
{
	os << "\\" << char_;
}


void MathSpecialCharInset::writeNormal(std::ostream & os) const
{
	os << "\\" << char_;
}
