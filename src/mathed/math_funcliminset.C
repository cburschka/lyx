#include "math_funcliminset.h"
#include "support.h"
#include "support/LOstream.h"


using std::ostream;

MathFuncLimInset::MathFuncLimInset(string const & name)
	: name_(name)
{}


MathInset * MathFuncLimInset::clone() const
{
	return new MathFuncLimInset(*this);
}


bool MathFuncLimInset::isScriptable() const
{
	return mi_.style == LM_ST_DISPLAY;
}


void MathFuncLimInset::write(MathWriteInfo & os) const
{
	os << '\\' << name_ << ' ';
}


void MathFuncLimInset::writeNormal(ostream & os) const
{
	os << "[" << name_ << "] ";
}


void MathFuncLimInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	mathed_string_dim(LM_TC_TEXTRM, mi_, name_, ascent_, descent_, width_);
}


void MathFuncLimInset::draw(Painter & pain, int x, int y) const
{  
	drawStr(pain, LM_TC_TEXTRM, mi_, x, y, name_);
}
