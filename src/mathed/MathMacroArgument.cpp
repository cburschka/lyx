/**
 * \file MathMacroArgument.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathMacroArgument.h"
#include "MathStream.h"
#include "MathSupport.h"

#include "support/debug.h"


namespace lyx {

MathMacroArgument::MathMacroArgument(int n)
	: number_(n)
{
	if (n < 1 || n > 9) {
		LYXERR0("MathMacroArgument::MathMacroArgument: wrong Argument id: " << n);
	}

	// The profiler tells us not to use
	// str_ = '#' + convert<docstring>(n);
	// so we do the conversion of n to ASCII manually.
	// This works because 1 <= n <= 9.
	str_.resize(2);
	str_[0] = '#';
	str_[1] = '0' + n;
}


Inset * MathMacroArgument::clone() const
{
	return new MathMacroArgument(*this);
}


void MathMacroArgument::setNumber(int n)
{
	if (n < 1 || n > 9) {
		LYXERR0("MathMacroArgument::setNumber: wrong Argument id: " << n);
	}

	number_ = n;
	str_[1] = '0' + n;
}


void MathMacroArgument::write(WriteStream & os) const
{
	os << str_;
}


void MathMacroArgument::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_string_dim(mi.base.font, str_, dim);
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


} // namespace lyx
