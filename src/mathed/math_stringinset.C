/**
 * \file math_stringinset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_stringinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "debug.h"

using std::auto_ptr;


MathStringInset::MathStringInset(string const & s)
	: str_(s)
{}


auto_ptr<InsetBase> MathStringInset::clone() const
{
	return auto_ptr<InsetBase>(new MathStringInset(*this));
}


void MathStringInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, str_, dim);
}


void MathStringInset::draw(PainterInfo & pi, int x, int y) const
{
	//lyxerr << "drawing '" << str_ << "' code: " << code_ << endl;
	drawStr(pi, pi.base.font, x, y, str_);
}


void MathStringInset::normalize(NormalStream & os) const
{
	os << "[string " << str_ << ' ' << "mathalpha" << ']';
}


void MathStringInset::maple(MapleStream & os) const
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


void MathStringInset::mathematica(MathematicaStream & os) const
{
	os << ' ' << str_ << ' ';
}


void MathStringInset::octave(OctaveStream & os) const
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
