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
	: MathInset(nm)
{}


MathInset * MathFuncInset::clone() const
{
	return new MathFuncInset(*this);
}


void MathFuncInset::write(std::ostream & os, bool /* fragile */) const
{
	os << "\\" << name_ << ' ';
}


void MathFuncInset::writeNormal(std::ostream & os) const
{
	os << "[" << name_ << "] ";
}


void MathFuncInset::metrics(MathStyles st) 
{
	size_ = st;
	if (name_.empty()) 
		mathed_char_dim(LM_TC_TEXTRM, size_, 'I', ascent_, descent_, width_);
	else 
		mathed_string_dim(LM_TC_TEXTRM, size_, name_, ascent_, descent_, width_);
}


void MathFuncInset::draw(Painter & pain, int x, int y)
{ 
	xo(x);
	yo(y);
	if (name_.empty()) 
		drawChar(pain, LM_TC_TEXTRM, size_, x, y, ' ');
	else
		drawStr(pain, LM_TC_TEXTRM, size_, x, y, name_);
}
