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


MathUnknownInset::MathUnknownInset(string const & nm, bool final, bool black)
	: name_(nm), final_(final), black_(black)
{}


auto_ptr<InsetBase> MathUnknownInset::clone() const
{
	return auto_ptr<InsetBase>(new MathUnknownInset(*this));
}


string MathUnknownInset::name() const
{
	return name_;
}


void MathUnknownInset::setName(string const & name)
{
	name_ = name;
}


bool MathUnknownInset::match(MathAtom const & at) const
{
	MathUnknownInset const * q = at->asUnknownInset();
	return q && name_ == q->name_;
}


void MathUnknownInset::normalize(NormalStream & os) const
{
	os << "[unknown " << name_ << ']';
}


void MathUnknownInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, name_, dim);
}


void MathUnknownInset::draw(PainterInfo & pi, int x, int y) const
{
	if (black_)
		drawStrBlack(pi, x, y, name_);
	else
		drawStrRed(pi, x, y, name_);
}


void MathUnknownInset::finalize()
{
	final_ = true;
}


bool MathUnknownInset::final() const
{
	return final_;
}


void MathUnknownInset::maple(MapleStream & os) const
{
	os << name_;
}

void MathUnknownInset::mathematica(MathematicaStream & os) const
{
	os << name_;
}


void MathUnknownInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mi") << name_ << ETag("mi");
}


void MathUnknownInset::octave(OctaveStream & os) const
{
	os << name_;
}
