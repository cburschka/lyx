#include "math_exfuncinset.h"
#include "support.h"
#include "debug.h"
#include "support/LOstream.h"


using std::ostream;


MathExFuncInset::MathExFuncInset(string const & name)
	: MathNestInset(1), name_(name)
{}


MathInset * MathExFuncInset::clone() const
{
	return new MathExFuncInset(*this);
}


void MathExFuncInset::write(MathWriteInfo & os) const
{
	os << '\\' << name_ << '{';
	cell(0).write(os);
	os << '}';
}


void MathExFuncInset::writeNormal(ostream & os) const
{
	os << "[" << name_ << ' ';
	cell(0).writeNormal(os);
	os << "] ";
}


void MathExFuncInset::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	mathed_string_dim(LM_TC_TEXTRM, mi_, name_, ascent_, descent_, width_);
	lyxerr << "should not happen\n";
}


void MathExFuncInset::draw(Painter & pain, int x, int y) const
{  
	drawStr(pain, LM_TC_TEXTRM, mi_, x, y, name_);
	lyxerr << "should not happen\n";
}


string MathExFuncInset::octavize() const
{
	return name_ + '(' + cell(0).octavize() + ')';
}
