#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_stringinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "LColor.h"
#include "Painter.h"
#include "math_support.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "debug.h"

#include <cctype>


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
	return mathed_string_ascent(font_, str_);
}


int MathStringInset::descent() const
{
	return mathed_string_descent(font_, str_);
}


int MathStringInset::width() const
{
	return mathed_string_width(font_, str_);
}


void MathStringInset::validate(LaTeXFeatures & features) const
{
	//lyxerr << "stringinset amssymb" << endl;
	if (code_ == LM_TC_MSA || code_ == LM_TC_MSB)
		features.require("amssymb");
}


void MathStringInset::metrics(MathMetricsInfo const & mi) const
{
	whichFont(font_, code_, mi);
}


void MathStringInset::draw(Painter & pain, int x, int y) const
{
	//lyxerr << "drawing '" << str_ << "' code: " << code_ << endl;
	drawStr(pain, font_, x, y, str_);
}


void MathStringInset::normalize(NormalStream & os) const
{
	os << "[string " << str_ << ' ' << "mathalpha" << "]";
}


void MathStringInset::maplize(MapleStream & os) const
{
	if (code_ != LM_TC_VAR || str_.size() <= 1) {
		os << ' ' << str_ << ' ';
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
		os << ' ' << str_ << ' ';
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
		os << "<mi> " << str_ << " </mi>";
	else if (code_ == LM_TC_CONST)
		os << "<mn> " << str_ << " </mn>";
	else if (code_ == LM_TC_RM || code_ == LM_TC_TEXTRM)
		os << "<mtext> " << str_ <<  " </mtext>";
	else
		os << str_;
}


void MathStringInset::write(WriteStream & os) const
{
	if (math_font_name(code_))
		os << '\\' << math_font_name(code_) << '{' << str_ << '}';
	else
		os << str_;
}
