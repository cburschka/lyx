#include <config.h>

#include "math_funcliminset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"


MathFuncLimInset::MathFuncLimInset(string const & name)
	: name_(name)
{}


MathInset * MathFuncLimInset::clone() const
{
	return new MathFuncLimInset(*this);
}


bool MathFuncLimInset::isScriptable() const
{
	//return mi_.style == LM_ST_DISPLAY;
	return true;
}


void MathFuncLimInset::write(WriteStream & os) const
{
	os << '\\' << name_ << ' ';
}


void MathFuncLimInset::normalize(NormalStream & os) const
{
	os << "[funclim " << name_ << ']';
}


void MathFuncLimInset::metrics(MathMetricsInfo const & mi) const
{
	whichFont(font_, LM_TC_TEXTRM, mi);
	mathed_string_dim(font_, name_, ascent_, descent_, width_);
}


void MathFuncLimInset::draw(Painter & pain, int x, int y) const
{
	drawStr(pain, font_, x, y, name_);
}
