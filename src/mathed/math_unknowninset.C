#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_unknowninset.h"
#include "font.h"
#include "Painter.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


extern LyXFont WhichFont(short type, int size);


MathUnknownInset::MathUnknownInset(string const & nm)
	: name_(nm)
{}


MathInset * MathUnknownInset::clone() const
{
	return new MathUnknownInset(*this);
}


string const & MathUnknownInset::name() const
{
	return name_;
}


void MathUnknownInset::setName(string const & n)
{
	name_ = n;
}


bool MathUnknownInset::match(MathInset * p) const
{
	MathUnknownInset const * q = p->asUnknownInset();
	return q && name_ == q->name_;
}


void MathUnknownInset::write(WriteStream & os) const
{
	os << "\\" << name_ << ' ';
}


void MathUnknownInset::normalize(NormalStream & os) const
{
	os << "[unknown " << name_ << ']';
}


void MathUnknownInset::metrics(MathMetricsInfo const & mi) const
{
	whichFont(font_, LM_TC_TEX, mi);
	mathed_string_dim(font_, name_, ascent_, descent_, width_);
}


void MathUnknownInset::draw(Painter & pain, int x, int y) const
{
	drawStr(pain, font_, x, y, name_);
}


void MathUnknownInset::maplize(MapleStream & os) const
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
