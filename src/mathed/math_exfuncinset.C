#include <config.h>

#include "math_exfuncinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathExFuncInset::MathExFuncInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathExFuncInset::MathExFuncInset(string const & name, MathArray const & ar)
	: MathNestInset(1), name_(name)
{
	cell(0) = ar;
}


MathInset * MathExFuncInset::clone() const
{
	return new MathExFuncInset(*this);
}


void MathExFuncInset::metrics(MathMetricsInfo & mi) const
{
	mathed_string_dim(mi.base.font, name_, ascent_, descent_, width_);
}


void MathExFuncInset::draw(MathPainterInfo & pi, int x, int y) const
{
	drawStrBlack(pi, x, y, name_);
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
