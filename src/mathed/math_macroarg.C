#include <config.h>

#include "math_macroarg.h"
#include "mathed/support.h"
#include "debug.h"


MathMacroArgument::MathMacroArgument()
	: expnd_mode_(false), number_(1)
{
	SetType(LM_OT_MACRO_ARG);
}


MathMacroArgument::MathMacroArgument(int n)
	: expnd_mode_(false), number_(n)
{
	SetType(LM_OT_MACRO_ARG);
}


MathMacroArgument::~MathMacroArgument()
{
	lyxerr << "help, destroyme!" << std::endl;
}


MathedInset * MathMacroArgument::Clone()
{
	return this;
}


void MathMacroArgument::setNumber(int n)
{
	number_ = n;
}


void MathMacroArgument::setExpand(bool e)
{
	expnd_mode_ = e;
}


bool MathMacroArgument::getExpand() const
{
	return expnd_mode_;
}


void MathMacroArgument::draw(Painter & pain, int x, int baseline)
{
	if (expnd_mode_) {
		MathParInset::draw(pain, x, baseline);
	}
	else {
		std::ostringstream ost;
		ost << '#' << number_;
		drawStr(pain, LM_TC_TEX, size, x, baseline, ost.str().c_str());
	}
}

void MathMacroArgument::Metrics()
{
	if (expnd_mode_) {
		MathParInset::Metrics();
	} else {
		std::ostringstream ost;
		ost << '#' << number_;
		width = mathed_string_width(LM_TC_TEX, size, ost.str().c_str());
		mathed_string_height(LM_TC_TEX, size, ost.str().c_str(),
				     ascent, descent);
	}
}


void MathMacroArgument::Write(ostream & os, bool fragile)
{
	if (expnd_mode_) {
		MathParInset::Write(os, fragile);
	} else {
		os << '#' << number_ << ' ';
	}
}
