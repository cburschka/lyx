/**
 * \file InsetMathClass.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathClass.h"

#include "support/docstream.h"


namespace lyx {

InsetMathClass::InsetMathClass(Buffer * buf, MathClass mc)
	: InsetMathNest(buf, 1), math_class_(mc)
{}


Inset * InsetMathClass::clone() const
{
	return new InsetMathClass(*this);
}


Limits InsetMathClass::defaultLimits(bool display) const
{
	if (allowsLimitsChange() && display)
		return LIMITS;
	else
		return NO_LIMITS;
}


void InsetMathClass::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi, dim);
}


void InsetMathClass::draw(PainterInfo & pi, int x, int y) const
{
	cell(0).draw(pi, x, y);
}


void InsetMathClass::write(TeXMathStream & os) const
{
	InsetMathNest::write(os);
	writeLimits(os);
}


docstring InsetMathClass::name() const
{
	return class_to_string(math_class_);
}


void InsetMathClass::infoize(odocstream & os) const
{
	os << name() << " ";
}


} // namespace lyx
