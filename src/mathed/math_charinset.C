#ifdef __GNUG__
#pragma implementation
#endif

#include "math_charinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support/LOstream.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "debug.h"


MathCharInset::MathCharInset(char c, MathTextCodes t)
	: char_(c)
{
	code(t);
}


MathInset * MathCharInset::clone() const
{   
	return new MathCharInset(*this);
}


int MathCharInset::ascent() const
{
	return mathed_char_ascent(code_, size(), char_);
}


int MathCharInset::descent() const
{
	return mathed_char_descent(code_, size(), char_);
}


int MathCharInset::width() const
{
	return mathed_char_width(code_, size(), char_);
}


void MathCharInset::metrics(MathStyles st) const
{
	size_ = st;
}


void MathCharInset::draw(Painter & pain, int x, int y) const
{ 
	xo(x);
	yo(y);
	drawChar(pain, code_, size_, x, y, char_);
}


void MathCharInset::write(std::ostream & os, bool) const
{
	if (code_ >= LM_TC_RM && code_ <= LM_TC_TEXTRM) 
		os << '\\' << math_font_name[code_ - LM_TC_RM] << '{';

	if ((code_ == LM_TC_TEX && char_ != '{' && char_ != '}') ||
			(code_ == LM_TC_SPECIAL))
		os << '\\';

	os << char_;

	if (code_ >= LM_TC_RM && code_ <= LM_TC_TEXTRM)
		os << '}';
}


void MathCharInset::writeNormal(std::ostream & os) const
{
	os << char_;
}


bool MathCharInset::isRelOp() const
{
	return char_ == '=' || char_ == '<' || char_ == '>';
}
