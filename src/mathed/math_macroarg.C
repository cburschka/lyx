#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macroarg.h"
#include "math_macro.h"
#include "math_defs.h"
#include "mathed/support.h"
#include "debug.h"



MathMacroArgument::MathMacroArgument(int n)
	: MathNestInset(1), number_(n), expanded_(false)
{
	if (n < 1 || n > 9) {
		lyxerr << "MathMacroArgument::MathMacroArgument: wrong Argument id: "
			<< n << std::endl;
	}
	str_[0] = '#';
	str_[1] = '0' + n;
	str_[2] = '\0';
}


MathInset * MathMacroArgument::clone() const
{
	return new MathMacroArgument(*this);
}


void MathMacroArgument::write(std::ostream & os, bool /*fragile*/) const
{
	os << '#' << number_;
}


void MathMacroArgument::metrics(MathStyles st) const
{
	if (expanded_) {
		xcell(0).metrics(st);
		width_   = xcell(0).width();
		ascent_  = xcell(0).ascent();
		descent_ = xcell(0).descent();
	} else
		mathed_string_dim(LM_TC_TEX, size(), str_, width_, ascent_, descent_);
}


void MathMacroArgument::draw(Painter & pain, int x, int y) const
{
	if (expanded_)
		xcell(0).draw(pain, x, y);
	else
		drawStr(pain, LM_TC_TEX, size(), x, y, str_);
}


void MathMacroArgument::writeNormal(std::ostream & os) const
{
	os << "[macroarg " << number_ << "] ";
}


void MathMacroArgument::substitute(MathMacro const & m)
{
	cell(0) = m.cell(number_ - 1);
	expanded_ = true;
}

