#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macroarg.h"
#include "math_macro.h"
#include "math_defs.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "debug.h"



MathMacroArgument::MathMacroArgument(int n, MathTextCodes code)
	: MathNestInset(1), number_(n), expanded_(false), code_(code)
{
	if (n < 1 || n > 9) {
		lyxerr << "MathMacroArgument::MathMacroArgument: wrong Argument id: "
			<< n << std::endl;
	}
	str_[0] = '#';
	str_[1] = static_cast<unsigned char>('0' + n);
	str_[2] = '\0';
}


MathInset * MathMacroArgument::clone() const
{
	return new MathMacroArgument(*this);
}


void MathMacroArgument::write(WriteStream & os) const
{
	if (code_ == LM_TC_MIN)
		os << str_;
	else
		os << '\\' << math_font_name(code_) << '{' << str_ << '}';
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


void MathMacroArgument::normalize(NormalStream & os) const
{
	os << "[macroarg " << str_ << "] ";
}


void MathMacroArgument::substitute(MathMacro const & m)
{
	cell(0) = m.cell(number_ - 1);
	if (code_ != LM_TC_MIN)
		for (MathArray::iterator it = cell(0).begin(); it != cell(0).end(); ++it)
			it->nucleus()->handleFont(code_);
	expanded_ = true;
}

