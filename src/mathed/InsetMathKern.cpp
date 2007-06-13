/**
 * \file InsetMathKern.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathKern.h"
#include "MathStream.h"
#include "MathStream.h"
#include "MathSupport.h"
#include "Dimension.h"


namespace lyx {

using std::string;
using std::auto_ptr;


InsetMathKern::InsetMathKern()
{
}


InsetMathKern::InsetMathKern(Length const & w)
	: wid_(w)
{
}


InsetMathKern::InsetMathKern(docstring const & s)
	: wid_(to_utf8(s))
{
}


auto_ptr<Inset> InsetMathKern::doClone() const
{
	return auto_ptr<Inset>(new InsetMathKern(*this));
}


bool InsetMathKern::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = 0;
	dim.des = 0;
	dim.wid = wid_.inPixels(0, mathed_char_width(mi.base.font, 'M'));
	if (dim == dim_)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathKern::draw(PainterInfo &, int, int) const
{}


void InsetMathKern::write(WriteStream & os) const
{
	os << "\\kern" << from_utf8(wid_.asLatexString()) << ' ';
}


void InsetMathKern::normalize(NormalStream & os) const
{
	os << "[kern " << from_utf8(wid_.asLatexString()) << ']';
}


} // namespace lyx
