#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

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


bool MathUnknownInset::match(MathInset * p) const
{
	MathUnknownInset const * q = p->asUnknownInset();
	return q && name_ == q->name_;
}


void MathUnknownInset::normalize(NormalStream & os) const
{
	os << "[unknown " << name_ << ']';
}


void MathUnknownInset::metrics(MathMetricsInfo & mi) const
{
	mathed_string_dim(mi.base.font, name_, dim_);
}


void MathUnknownInset::draw(MathPainterInfo & pi, int x, int y) const
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


void MathUnknownInset::maplize(MapleStream & os) const
{
	os << name_;
}

void MathUnknownInset::mathematicize(MathematicaStream & os) const
{
	os << name_;
}


void MathUnknownInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mi") << name_ << ETag("mi");
}


void MathUnknownInset::octavize(OctaveStream & os) const
{
	os << name_;
}
