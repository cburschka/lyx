#ifdef __GNUG__
#pragma implementation
#endif

#include <cctype>

#include "math_stringinset.h"
#include "math_mathmlstream.h"
#include "LColor.h"
#include "Painter.h"
#include "math_support.h"
#include "math_parser.h"
#include "debug.h"


MathStringInset::MathStringInset()
	: str_(), code_(LM_TC_MIN)
{}

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


void MathStringInset::write(WriteStream & os) const
{
	if (math_font_name(code_)) 
		os << '\\' << math_font_name(code_) << '{' << str_.c_str() << '}';
	else 
		os << str_.c_str();
}


void MathStringInset::normalize(NormalStream & os) const
{
	os << "[string " << str_.c_str() << ' ' << "mathalpha" << "]";
}


void MathStringInset::maplize(MapleStream & os) const
{
	if (code_ != LM_TC_VAR || str_.size() <= 1) {
		os << str_.c_str();
		return;
	}	

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (string::size_type i = 1; i < str_.size(); ++i) 
		os << '*' << str_[i];
}


void MathStringInset::octavize(OctaveStream & os) const
{
	if (code_ != LM_TC_VAR || str_.size() <= 1) {
		os << str_.c_str();
		return;
	}	

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (string::size_type i = 1; i < str_.size(); ++i) 
		os << '*' << str_[i];
}


void MathStringInset::mathmlize(MathMLStream & os) const
{
	if (code_ == LM_TC_VAR)
		os << "<mi> " << str_.c_str() << " </mi>";
	else if (code_ == LM_TC_CONST)
		os << "<mn> " << str_.c_str() << " </mn>";
	else if (code_ == LM_TC_RM || code_ == LM_TC_TEXTRM)
		os << "<mtext> " << str_.c_str() <<  " </mtext>";
	else
		os << str_.c_str();
}
