/**
 * \file MathMacroArgument.C
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
#include "InsetMathMacro.h"
#include "MathMLStream.h"
#include "MathSupport.h"
#include "debug.h"


namespace lyx {

using std::endl;
using std::auto_ptr;
using std::size_t;
using std::vector;


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


auto_ptr<InsetBase> MathMacroArgument::doClone() const
{
	return auto_ptr<InsetBase>(new MathMacroArgument(*this));
}


void MathMacroArgument::write(WriteStream & os) const
{
	os << str_;
}


void MathMacroArgument::metrics(MetricsInfo & mi, Dimension & dim) const
{
	vector<char_type> n(str_, str_ + 3);
	mathed_string_dim(mi.base.font, n, dim_);
	dim = dim_;
}


void MathMacroArgument::draw(PainterInfo & pi, int x, int y) const
{
	// FIXME UNICODE
	drawStrRed(pi, x, y, from_utf8(str_));
	setPosCache(pi, x, y);
}


void MathMacroArgument::normalize(NormalStream & os) const
{
	os << "[macroarg " << str_ << "] ";
}


} // namespace lyx
