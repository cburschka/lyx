#ifdef __GNUG__
#pragma implementation
#endif

#include "math_macroarg.h"
#include "math_macro.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "debug.h"


using std::endl;


MathMacroArgument::MathMacroArgument(int n)
	: MathNestInset(1), number_(n), expanded_(false)
{
	if (n < 1 || n > 9) {
		lyxerr << "MathMacroArgument::MathMacroArgument: wrong Argument id: "
			<< n << endl;
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
	os << str_;
}


void MathMacroArgument::metrics(MathMetricsInfo & mi) const
{
	if (expanded_)
		dim_ = cell(0).metrics(mi);
	else
		mathed_string_dim(mi.base.font, str_, dim_);
}


void MathMacroArgument::draw(MathPainterInfo & pi, int x, int y) const
{
	if (expanded_)
		cell(0).draw(pi, x, y);
	else
		drawStrRed(pi, x, y, str_);
}


void MathMacroArgument::normalize(NormalStream & os) const
{
	os << "[macroarg " << str_ << "] ";
}


void MathMacroArgument::substitute(MathMacro const & m)
{
	cell(0) = m.cell(number_ - 1);
	expanded_ = true;
}
