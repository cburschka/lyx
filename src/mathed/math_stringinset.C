#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>

#include "math_stringinset.h"
#include "LColor.h"
#include "Painter.h"
#include "support/LOstream.h"
#include "support.h"
#include "math_parser.h"
#include "debug.h"


MathStringInset::MathStringInset(string const & s, MathTextCodes t)
	: str_(s), code_(t)
{}


MathInset * MathStringInset::clone() const
{   
	return new MathStringInset(*this);
}


int MathStringInset::ascent() const
{
	return mathed_string_ascent(code_, mi_, str_);
}


int MathStringInset::descent() const
{
	return mathed_string_descent(code_, mi_, str_);
}


int MathStringInset::width() const
{
	return mathed_string_width(code_, mi_, str_);
}


void MathStringInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
}


void MathStringInset::draw(Painter & pain, int x, int y) const
{ 
	//lyxerr << "drawing '" << str_ << "' code: " << code_ << endl;
	drawStr(pain, code_, mi_, x, y, str_);
}


void MathStringInset::write(MathWriteInfo & os) const
{
	if (math_font_name(code_)) 
		os << '\\' << math_font_name(code_) << '{' << str_ << '}';
	else 
		os << str_;
}


void MathStringInset::writeNormal(std::ostream & os) const
{
	os << "[string " << str_ << " " << "mathalpha" << "]";
}
