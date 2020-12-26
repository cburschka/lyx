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
#include "MathSupport.h"

#include "Dimension.h"
#include "MetricsInfo.h"

#include "support/docstring.h"

namespace lyx {

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


Inset * InsetMathKern::clone() const
{
	return new InsetMathKern(*this);
}


void InsetMathKern::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.asc = 0;
	dim.des = 0;
	dim.wid = mi.base.inPixels(wid_);
}


void InsetMathKern::draw(PainterInfo &, int, int) const
{}


void InsetMathKern::write(TeXMathStream & os) const
{
	if (wid_.empty())
		os << "\\kern" << ' ';
	else if (wid_.unit() == Length::MU)
		os << "\\mkern" << from_utf8(wid_.asLatexString()) << ' ';
	else
		os << "\\kern" << from_utf8(wid_.asLatexString()) << ' ';
}


void InsetMathKern::normalize(NormalStream & os) const
{
	if (wid_.empty())
		os << "[kern]";
	else
		os << "[kern " << from_utf8(wid_.asLatexString()) << ']';
}


void InsetMathKern::infoize2(odocstream & os) const
{
	os << "Kern";
	if (!wid_.empty())
		os << ": " << from_utf8(wid_.asLatexString());
}


} // namespace lyx
