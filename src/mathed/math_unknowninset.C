/**
 * \file math_unknowninset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_unknowninset.h"
#include "math_support.h"
#include "math_atom.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"

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
	mathed_string_dim(mi.base.font, name_, dim);
	dim_ = dim;
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


void InsetMathUnknown::mathmlize(MathMLStream & os) const
{
	os << MTag("mi") << name_ << ETag("mi");
}


void InsetMathUnknown::octave(OctaveStream & os) const
{
	os << name_;
}
