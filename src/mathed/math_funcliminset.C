#include "math_funcliminset.h"
#include "mathed/math_parser.h"
#include "mathed/support.h"
#include "support/LOstream.h"


using std::ostream;

MathFuncLimInset::MathFuncLimInset(const latexkeys * l)
	: sym_(l)
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
	os << '\\' << sym_->name << ' ';
}


void MathFuncLimInset::writeNormal(ostream & os) const
{
	os << "[" << sym_->name << "] ";
}


void MathFuncLimInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	mathed_string_dim(LM_TC_TEXTRM, mi_, sym_->name, ascent_, descent_, width_);
}


void MathFuncLimInset::draw(Painter & pain, int x, int y) const
{  
	xo(x);
	yo(y);
	drawStr(pain, LM_TC_TEXTRM, mi_, x, y, sym_->name);
}
