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


void MathFuncInset::metrics(MathMetricsInfo const & st) const 
{
	size_ = st;
	mathed_string_dim(LM_TC_TEX, size_.size, name_, ascent_, descent_, width_);
}


void MathFuncInset::draw(Painter & pain, int x, int y) const
{ 
	xo(x);
	yo(y);
	drawStr(pain, LM_TC_TEX, size_.size, x, y, name_);
}
