#ifdef __GNUG__
#pragma implementation
#endif

#include "math_unknowninset.h"
#include "font.h"
#include "Painter.h"
#include "math_support.h"
#include "math_mathmlstream.h"



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


void MathUnknownInset::write(WriteStream & os) const
{
	os << "\\" << name_.c_str() << ' ';
}


void MathUnknownInset::normalize(NormalStream & os) const
{
	os << "[func " << name_.c_str() << ']';
}


void MathUnknownInset::metrics(MathMetricsInfo const & mi) const 
{
	mi_ = mi;
	mathed_string_dim(LM_TC_TEX, mi_, name_, ascent_, descent_, width_);
}


void MathUnknownInset::draw(Painter & pain, int x, int y) const
{ 
	drawStr(pain, LM_TC_TEX, mi_, x, y, name_);
}


void MathUnknownInset::maplize(MapleStream & os) const
{
	os << name_.c_str();
}


void MathUnknownInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mi") << name_.c_str() << ETag("mi");
}


void MathUnknownInset::octavize(OctaveStream & os) const
{
	os << name_.c_str();
}
