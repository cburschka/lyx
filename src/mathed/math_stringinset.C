#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_stringinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "LColor.h"
#include "math_support.h"
#include "math_parser.h"
#include "LaTeXFeatures.h"
#include "debug.h"


MathStringInset::MathStringInset(string const & s)
	: str_(s)
{}


MathInset * MathStringInset::clone() const
{
	return new MathStringInset(*this);
}


void MathStringInset::metrics(MathMetricsInfo & mi) const
{
	mathed_string_dim(mi.base.font, str_, ascent_, descent_, width_);
}


void MathStringInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//lyxerr << "drawing '" << str_ << "' code: " << code_ << endl;
	drawStr(pi, pi.base.font, x, y, str_);
}


void MathStringInset::normalize(NormalStream & os) const
{
	os << "[string " << str_ << ' ' << "mathalpha" << "]";
}


void MathStringInset::maplize(MapleStream & os) const
{
	if (/*code_ != LM_TC_VAR || */ str_.size() <= 1) {
		os << ' ' << str_ << ' ';
		return;
	}

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (string::size_type i = 1; i < str_.size(); ++i)
		os << str_[i];
}


void MathStringInset::octavize(OctaveStream & os) const
{
	if (/*code_ != LM_TC_VAR ||*/ str_.size() <= 1) {
		os << ' ' << str_ << ' ';
		return;
	}

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (string::size_type i = 1; i < str_.size(); ++i)
		os << str_[i];
}


void MathStringInset::mathmlize(MathMLStream & os) const
{
/*
	if (code_ == LM_TC_VAR)
		os << "<mi> " << str_ << " </mi>";
	else if (code_ == LM_TC_CONST)
		os << "<mn> " << str_ << " </mn>";
	else if (code_ == LM_TC_RM || code_ == LM_TC_TEXTRM)
		os << "<mtext> " << str_ <<  " </mtext>";
	else
*/
		os << str_;
}


void MathStringInset::write(WriteStream & os) const
{
	os << str_;
}
