/**
 * \file InsetMathNumber.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathNumber.h"
#include "MathMLStream.h"
#include "MathStream.h"
#include "MathSupport.h"

using lyx::docstring;

using std::string;
using std::auto_ptr;


InsetMathNumber::InsetMathNumber(string const & s)
	: str_(s)
{}


auto_ptr<InsetBase> InsetMathNumber::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathNumber(*this));
}


void InsetMathNumber::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// FIXME UNICODE
	mathed_string_dim(mi.base.font, lyx::from_utf8(str_), dim);
}


void InsetMathNumber::draw(PainterInfo & pi, int x, int y) const
{
	// FIXME UNICODE
	pi.draw(x, y, lyx::from_utf8(str_));
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


void InsetMathNumber::mathmlize(MathMLStream & os) const
{
	os << "<mi> " << str_ << " </mi>";
}


void InsetMathNumber::write(WriteStream & os) const
{
	os << str_;
}
