#ifdef __GNUG__
#pragma implementation
#endif

#include "math_specialcharinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"


MathSpecialCharInset::MathSpecialCharInset(char c)
	: char_(c)
{}


MathInset * MathSpecialCharInset::clone() const
{
	return new MathSpecialCharInset(*this);
}


int MathSpecialCharInset::ascent() const
{
	return mathed_char_ascent(font_, char_);
}


int MathSpecialCharInset::descent() const
{
	return mathed_char_descent(font_, char_);
}


int MathSpecialCharInset::width() const
{
	return mathed_char_width(font_, char_);
}


void MathSpecialCharInset::metrics(MathMetricsInfo const & mi) const
{
	whichFont(font_, LM_TC_CONST, mi);
}


void MathSpecialCharInset::draw(Painter & pain, int x, int y) const
{
	drawChar(pain, font_, x, y, char_);
}


void MathSpecialCharInset::write(WriteStream & os) const
{
	os << "\\" << char_;
}


void MathSpecialCharInset::normalize(NormalStream & os) const
{
	os << "\\" << char_;
}
