#include <config.h>

#include "math_unknowninset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathUnknownInset::MathUnknownInset(string const & nm, bool final, bool black)
	: name_(nm), final_(final), black_(black)
{}


MathInset * MathUnknownInset::clone() const
{
	return new MathUnknownInset(*this);
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


Dimension MathUnknownInset::metrics(MetricsInfo & mi) const
{
	Dimension dim;
	mathed_string_dim(mi.base.font, name_, dim);
	return dim;
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
