#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macro.h"
#include "math_macroarg.h"
#include "mathed/support.h"
#include "Lsstream.h"
#include "debug.h"


using namespace std;

MathMacroArgument::MathMacroArgument(int n)
	: MathedInset(string(), LM_OT_MACRO_ARG, LM_ST_TEXT),
		number_(n)
{
	if (n < 1 || n > 9) {
		lyxerr << "MathMacroArgument::MathMacroArgument: wrong Argument id: "
			<< n << endl;
		lyx::Assert(0);
	}
}

MathedInset * MathMacroArgument::Clone()
{
	//return new MathMacroArgument(*this);
	return this;
}

int MathMacroArgument::number() const
{
	return number_;
}

void MathMacroArgument::substitute(MathMacro * /*m*/)
{
	lyxerr << "Calling MathMacroArgument::substitute!\n";
	//return m->arg(number_)->Clone();
}


void MathMacroArgument::draw(Painter & pain, int x, int y)
{
	char str[] = "#0";
	str[1] += number_; 
	drawStr(pain, LM_TC_TEX, size(), x, y, str);
}


void MathMacroArgument::Metrics()
{
	char str[] = "#0";
	str[1] += number_; 
	width = mathed_string_width(LM_TC_TEX, size(), str);
	mathed_string_height(LM_TC_TEX, size(), str, ascent, descent);
}


void MathMacroArgument::Write(std::ostream & os, bool /*fragile*/)
{
	os << '#' << number_ << ' ';
}


void MathMacroArgument::WriteNormal(std::ostream & os)
{
	os << "{macroarg " << number_ << "} ";
}
