#include <config.h>

#include "math_exfuncinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathExFuncInset::MathExFuncInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathExFuncInset::clone() const
{
	return new MathExFuncInset(*this);
}


void MathExFuncInset::metrics(MathMetricsInfo const & mi) const
{
	whichFont(font_, LM_TC_TEXTRM, mi);
	mathed_string_dim(font_, name_, ascent_, descent_, width_);
}


void MathExFuncInset::draw(Painter & pain, int x, int y) const
{  
	drawStr(pain, font_, x, y, name_);
}


void MathExFuncInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ' << cell(0) << ']';
}


void MathExFuncInset::maplize(MapleStream & os) const
{
	os << name_ << '(' << cell(0) << ')';
}


void MathExFuncInset::mathmlize(MathMLStream & os) const
{
	os << MTag(name_.c_str()) << cell(0) << ETag(name_.c_str());
}


void MathExFuncInset::octavize(OctaveStream & os) const
{
	os << name_ << '(' << cell(0) << ')';
}


void MathExFuncInset::write(WriteStream & os) const
{
	os << '\\' << name_ << '{' << cell(0) << '}';
}


