#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macroarg.h"
#include "math_macro.h"
#include "math_defs.h"
#include "mathed/support.h"
#include "debug.h"



MathMacroArgument::MathMacroArgument(int n)
	: number_(n)
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


void MathMacroArgument::draw(Painter & pain, int x, int y)
{
	drawStr(pain, LM_TC_TEX, size(), x, y, str_);
}


int MathMacroArgument::ascent() const
{
	return mathed_char_ascent(LM_TC_TEX, size(), 'I');
}


int MathMacroArgument::descent() const
{
	return mathed_char_descent(LM_TC_TEX, size(), 'I');
}


int MathMacroArgument::width() const
{
	return mathed_string_width(LM_TC_TEX, size(), str_);
}


void MathMacroArgument::write(std::ostream & os, bool /*fragile*/) const
{
	os << '#' << number_ << ' ';
}


void MathMacroArgument::writeNormal(std::ostream & os) const
{
	os << "[macroarg " << number_ << "] ";
}


void MathMacroArgument::substitute(MathArray & array, MathMacro const & m) const
{
	array.push_back(m.cell(number_ - 1));
}

