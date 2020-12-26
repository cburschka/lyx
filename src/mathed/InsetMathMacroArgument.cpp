/**
 * \file InsetMathMacroArgument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetMathMacroArgument.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "support/debug.h"
#include "support/lassert.h"


namespace lyx {


Inset * InsetMathHash::clone() const
{
	return new InsetMathHash(*this);
}


void InsetMathHash::write(TeXMathStream & os) const
{
	os << str_;
}


void InsetMathHash::metrics(MetricsInfo & mi, Dimension & dim) const
{
	metricsStrRedBlack(mi, dim, str_);
}


void InsetMathHash::draw(PainterInfo & pi, int x, int y) const
{
	drawStrRed(pi, x, y, str_);
}


void InsetMathHash::normalize(NormalStream & os) const
{
	os << "[hash " << str_ << "] ";
}


InsetMathMacroArgument::InsetMathMacroArgument(int n)
	: number_(n)
{
	if (n < 1 || n > 9) {
		LYXERR0("InsetMathMacroArgument::InsetMathMacroArgument: wrong Argument id: "
		        << n);
		LASSERT(false, n = 1);
	}

	// The profiler tells us not to use
	// str_ = '#' + convert<docstring>(n);
	// so we do the conversion of n to ASCII manually.
	// This works because 1 <= n <= 9.
	str_.resize(2);
	str_[1] = '0' + n;
}


Inset * InsetMathMacroArgument::clone() const
{
	return new InsetMathMacroArgument(*this);
}


void InsetMathMacroArgument::setNumber(int n)
{
	if (n < 1 || n > 9) {
		LYXERR0("InsetMathMacroArgument::setNumber: wrong Argument id: " << n);
		LASSERT(false, return);
	}

	number_ = n;
	str_[1] = '0' + n;
}


void InsetMathMacroArgument::normalize(NormalStream & os) const
{
	os << "[macroarg " << str_ << "] ";
}


} // namespace lyx
