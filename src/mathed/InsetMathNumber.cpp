/**
 * \file InsetMathNumber.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathNumber.h"
#include "MathStream.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "MetricsInfo.h"

using namespace std;


namespace lyx {

InsetMathNumber::InsetMathNumber(docstring const & s)
	: str_(s)
{}


Inset * InsetMathNumber::clone() const
{
	return new InsetMathNumber(*this);
}


void InsetMathNumber::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, str_, dim);
}


void InsetMathNumber::draw(PainterInfo & pi, int x, int y) const
{
	pi.draw(x, y, str_);
}


void InsetMathNumber::normalize(NormalStream & os) const
{
	os << "[number " << str_ << ']';
}


void InsetMathNumber::maple(MapleStream & os) const
{
	os << str_;
}


void InsetMathNumber::mathematica(MathematicaStream & os) const
{
	os << str_;
}


void InsetMathNumber::octave(OctaveStream & os) const
{
	os << str_;
}


void InsetMathNumber::mathmlize(MathStream & os) const
{
	os << "<mn>" << str_ << "</mn>";
}


void InsetMathNumber::htmlize(HtmlStream & os) const
{
	os << str_;
}


void InsetMathNumber::write(WriteStream & os) const
{
	os << str_;
}


} // namespace lyx
