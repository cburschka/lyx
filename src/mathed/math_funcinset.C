#ifdef __GNUG__
#pragma implementation
#endif

#include "math_funcinset.h"
#include "font.h"
#include "Painter.h"
#include "math_support.h"
#include "math_mathmlstream.h"



extern LyXFont WhichFont(short type, int size);


MathFuncInset::MathFuncInset(string const & nm)
	: name_(nm)
{}


MathInset * MathFuncInset::clone() const
{
	return new MathFuncInset(*this);
}


string const & MathFuncInset::name() const
{
	return name_;
}


void MathFuncInset::setName(string const & n)
{
	name_ = n;
}


void MathFuncInset::metrics(MathMetricsInfo const & mi) const 
{
	mi_ = mi;
	mathed_string_dim(LM_TC_TEXTRM, mi_, name_, ascent_, descent_, width_);
}


void MathFuncInset::draw(Painter & pain, int x, int y) const
{ 
	drawStr(pain, LM_TC_TEXTRM, mi_, x, y, name_);
}


void MathFuncInset::maplize(MapleStream & os) const
{
	os << ' ' << name_.c_str();
}


void MathFuncInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mi") << name_.c_str() << ETag("mi");
}


void MathFuncInset::octavize(OctaveStream & os) const
{
	os << ' ' << name_.c_str();
}


void MathFuncInset::normalize(NormalStream & os) const
{
	os << "[func " << name_.c_str() << ']';
}


void MathFuncInset::write(WriteStream & os) const
{
	os << "\\" << name_.c_str() << ' ';
}
