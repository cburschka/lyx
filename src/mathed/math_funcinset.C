#ifdef __GNUG__
#pragma implementation
#endif

#include "math_funcinset.h"
#include "font.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"


using std::ostream;

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


void MathFuncInset::write(MathWriteInfo & os) const
{
	os << "\\" << name_ << ' ';
}


void MathFuncInset::writeNormal(std::ostream & os) const
{
	os << "[" << name_ << "] ";
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


string MathFuncInset::octavize() const
{
	return name_;
}
