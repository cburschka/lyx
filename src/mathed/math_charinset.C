#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>

#include "math_charinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support/LOstream.h"
#include "mathed/support.h"
#include "math_parser.h"
#include "debug.h"


MathCharInset::MathCharInset(char c)
	: char_(c), code_(nativeCode(c))
{
	if (isalpha(c))
		code_ = LM_TC_VAR;
}


MathCharInset::MathCharInset(char c, MathTextCodes t)
	: char_(c), code_((t == LM_TC_MIN) ? nativeCode(c) : t)
{}


MathTextCodes MathCharInset::nativeCode(char c) const
{
	if (isalpha(c))
		return LM_TC_VAR;
	return LM_TC_MIN;
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

	if (code_ == LM_TC_TEX || code_ == LM_TC_SPECIAL)
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


void MathCharInset::handleFont(MathTextCodes t)
{
	code_ = (code_ == t) ? LM_TC_VAR : t;
}
