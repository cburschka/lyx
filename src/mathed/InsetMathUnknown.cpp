/**
 * \file InsetMathUnknown.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathUnknown.h"
#include "MathSupport.h"
#include "MathAtom.h"
#include "MathStream.h"
#include "MathStream.h"


namespace lyx {

InsetMathUnknown::InsetMathUnknown(docstring const & nm, bool final, bool black)
	: name_(nm), final_(final), black_(black)
{}


Inset * InsetMathUnknown::clone() const
{
	return new InsetMathUnknown(*this);
}


docstring InsetMathUnknown::name() const
{
	return name_;
}


void InsetMathUnknown::setName(docstring const & name)
{
	name_ = name;
}


void InsetMathUnknown::normalize(NormalStream & os) const
{
	os << "[unknown " << name_ << ']';
}


bool InsetMathUnknown::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, name_, dim);
	docstring::const_reverse_iterator rit = name_.rbegin();
	kerning_ = mathed_char_kerning(mi.base.font, *rit);
	if (dim_ == dim)
		return false;
	dim_ = dim;
	return true;
}


void InsetMathUnknown::draw(PainterInfo & pi, int x, int y) const
{
	if (black_)
		drawStrBlack(pi, x, y, name_);
	else
		drawStrRed(pi, x, y, name_);
	setPosCache(pi, x, y);
}


void InsetMathUnknown::finalize()
{
	final_ = true;
}


bool InsetMathUnknown::final() const
{
	return final_;
}


void InsetMathUnknown::maple(MapleStream & os) const
{
	os << name_;
}


void InsetMathUnknown::mathematica(MathematicaStream & os) const
{
	os << name_;
}


void InsetMathUnknown::mathmlize(MathStream & os) const
{
	os << MTag("mi") << name_ << ETag("mi");
}


void InsetMathUnknown::octave(OctaveStream & os) const
{
	os << name_;
}


} // namespace lyx
