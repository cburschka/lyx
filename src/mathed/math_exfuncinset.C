#include "math_exfuncinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"


using std::ostream;


MathExFuncInset::MathExFuncInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathExFuncInset::clone() const
{
	return new MathExFuncInset(*this);
}


void MathExFuncInset::write(WriteStream & os) const
{
	os << '\\' << name_.c_str() << '{' << cell(0) << '}';
}


void MathExFuncInset::normalize(NormalStream & os) const
{
	os << '[' << name_.c_str() << ' ' << cell(0) << ']';
}


void MathExFuncInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	mathed_string_dim(LM_TC_TEXTRM, mi_, name_, ascent_, descent_, width_);
}


void MathExFuncInset::draw(Painter & pain, int x, int y) const
{  
	drawStr(pain, LM_TC_TEXTRM, mi_, x, y, name_);
}


void MathExFuncInset::octavize(OctaveStream & os) const
{
	os << name_.c_str() << '(' << cell(0) << ')';
}
