/**
 * \file math_macroarg.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_macroarg.h"
#include "math_macro.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "debug.h"

using std::endl;
using std::auto_ptr;
using std::size_t;


MathMacroArgument::MathMacroArgument(size_t n)
	: number_(n)
{
	if (n < 1 || n > 9) {
		lyxerr << "MathMacroArgument::MathMacroArgument: wrong Argument id: "
			<< n << endl;
	}
	str_[0] = '#';
	str_[1] = static_cast<unsigned char>('0' + n);
	str_[2] = '\0';
}


auto_ptr<InsetBase> MathMacroArgument::clone() const
{
	return auto_ptr<InsetBase>(new MathMacroArgument(*this));
}


void MathMacroArgument::write(WriteStream & os) const
{
	os << str_;
}


void MathMacroArgument::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, str_, dim_);
	dim = dim_;
}


void MathMacroArgument::draw(PainterInfo & pi, int x, int y) const
{
	drawStrRed(pi, x, y, str_);
	setPosCache(pi, x, y);
}


void MathMacroArgument::normalize(NormalStream & os) const
{
	os << "[macroarg " << str_ << "] ";
}
