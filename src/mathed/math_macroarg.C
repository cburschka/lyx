#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macroarg.h"
#include "math_macro.h"
#include "math_defs.h"
#include "math_mathmlstream.h"
#include "support.h"
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


void MathMacroArgument::write(MathWriteInfo & os) const
{
	os << '#' << number_;
}


void MathMacroArgument::metrics(MathMetricsInfo const & mi) const
{
	mi_ = mi;
	if (expanded_) {
		xcell(0).metrics(mi_);
		width_   = xcell(0).width();
		ascent_  = xcell(0).ascent();
		descent_ = xcell(0).descent();
	} else
		mathed_string_dim(LM_TC_TEX, mi_, str_, ascent_, descent_, width_);
}


void MathMacroArgument::draw(Painter & pain, int x, int y) const
{
	if (expanded_)
		xcell(0).draw(pain, x, y);
	else
		drawStr(pain, LM_TC_TEX, mi_, x, y, str_);
}


void MathMacroArgument::writeNormal(NormalStream & os) const
{
	os << "[macroarg " << number_ << "] ";
}


void MathMacroArgument::substitute(MathMacro const & m)
{
	cell(0) = m.cell(number_ - 1);
	expanded_ = true;
}

