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
	return scriptable_;
}


void MathFuncLimInset::write(WriteStream & os) const
{
	os << '\\' << name_ << ' ';
}


void MathFuncLimInset::normalize(NormalStream & os) const
{
	os << "[funclim " << name_ << ']';
}


void MathFuncLimInset::metrics(MathMetricsInfo & mi) const
{
	scriptable_ =  (mi.base.style == LM_ST_DISPLAY);
	mathed_string_dim(mi.base.font, name_, ascent_, descent_, width_);
}


void MathFuncLimInset::draw(MathPainterInfo & pi, int x, int y) const
{
	drawStrBlack(pi, x, y, name_);
}
