/**
 * \file InsetMathString.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathString.h"
#include "MathStream.h"
#include "MathStream.h"
#include "MathSupport.h"


namespace lyx {

InsetMathString::InsetMathString(docstring const & s)
	: str_(s)
{}


Inset * InsetMathString::clone() const
{
	return new InsetMathString(*this);
}


void InsetMathString::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, str_, dim);
}


void InsetMathString::draw(PainterInfo & pi, int x, int y) const
{
	pi.draw(x, y, str_);
}


void InsetMathString::normalize(NormalStream & os) const
{
	os << "[string " << str_ << ' ' << "mathalpha" << ']';
}


void InsetMathString::maple(MapleStream & os) const
{
	if (/*code_ != LM_TC_VAR ||*/ str_.size() <= 1) {
		os << ' ' << str_ << ' ';
		return;
	}

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (size_t i = 1; i < str_.size(); ++i)
		os << str_[i];
}


void InsetMathString::mathematica(MathematicaStream & os) const
{
	os << ' ' << str_ << ' ';
}


void InsetMathString::octave(OctaveStream & os) const
{
	if (/*code_ != LM_TC_VAR ||*/ str_.size() <= 1) {
		os << ' ' << str_ << ' ';
		return;
	}

	// insert '*' between adjacent chars if type is LM_TC_VAR
	os << str_[0];
	for (size_t i = 1; i < str_.size(); ++i)
		os << str_[i];
}


void InsetMathString::mathmlize(MathStream & os) const
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


void InsetMathString::write(WriteStream & os) const
{
	os << str_;
}


} // namespace lyx
