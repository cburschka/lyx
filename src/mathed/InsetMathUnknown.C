/**
 * \file InsetMathUnknown.C
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
#include "MathMLStream.h"
#include "MathStream.h"


namespace lyx {

using std::string;
using std::auto_ptr;


InsetMathUnknown::InsetMathUnknown(string const & nm, bool final, bool black)
	: name_(nm), final_(final), black_(black)
{}


auto_ptr<InsetBase> InsetMathUnknown::doClone() const
{
	return auto_ptr<InsetBase>(new InsetMathUnknown(*this));
}


string InsetMathUnknown::name() const
{
	return name_;
}


void InsetMathUnknown::setName(string const & name)
{
	name_ = name;
}


void InsetMathUnknown::normalize(NormalStream & os) const
{
	os << "[unknown " << name_ << ']';
}


void InsetMathUnknown::metrics(MetricsInfo & mi, Dimension & dim) const
{
	// FIXME UNICODE
	mathed_string_dim(mi.base.font, from_utf8(name_), dim);
	dim_ = dim;
}


void InsetMathUnknown::draw(PainterInfo & pi, int x, int y) const
{
	// FIXME UNICODE
	if (black_)
		drawStrBlack(pi, x, y, from_utf8(name_));
	else
		drawStrRed(pi, x, y, from_utf8(name_));
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


void InsetMathUnknown::mathmlize(MathMLStream & os) const
{
	os << MTag("mi") << name_ << ETag("mi");
}


void InsetMathUnknown::octave(OctaveStream & os) const
{
	os << name_;
}


} // namespace lyx
