#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macroarg.h"
#include "math_macro.h"
#include "math_defs.h"
#include "mathed/support.h"
#include "support/LAssert.h"
#include "debug.h"



MathMacroArgument::MathMacroArgument(int n)
	: number_(n)
{
	if (n < 1 || n > 9) {
		lyxerr << "MathMacroArgument::MathMacroArgument: wrong Argument id: "
			<< n << std::endl;
		lyx::Assert(0);
	}
}


MathInset * MathMacroArgument::Clone() const
{
	return new MathMacroArgument(*this);
}


void MathMacroArgument::draw(Painter & pain, int x, int y)
{
	char str[] = "#0";
	str[1] += number_; 
	drawStr(pain, LM_TC_TEX, size(), x, y, str);
}


void MathMacroArgument::Metrics(MathStyles st)
{
	char str[] = "#0";
	str[1] += number_; 
	size_ = st;
	mathed_string_dim(LM_TC_TEX, size(), str, ascent_, descent_, width_);
}


void MathMacroArgument::Write(std::ostream & os, bool /*fragile*/) const
{
	os << '#' << number_ << ' ';
}


void MathMacroArgument::WriteNormal(std::ostream & os) const
{
	os << "[macroarg " << number_ << "] ";
}


void MathMacroArgument::substitute(MathArray & arr, MathMacro const & m) const
{
	arr.push_back(m.cell(number_ - 1));
}

