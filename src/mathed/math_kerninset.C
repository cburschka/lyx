/**
 * \file math_kerninset.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "math_kerninset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "dimension.h"

using std::string;
using std::auto_ptr;


MathKernInset::MathKernInset()
{}


MathKernInset::MathKernInset(LyXLength const & w)
	: wid_(w)
{}


MathKernInset::MathKernInset(string const & s)
	: wid_(s)
{}


auto_ptr<InsetBase> MathKernInset::clone() const
{
	return auto_ptr<InsetBase>(new MathKernInset(*this));
}


void MathKernInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	dim.wid = wid_.inPixels(0, mathed_char_width(mi.base.font, 'M'));
	dim.asc = 0;
	dim.des = 0;
}


void MathKernInset::draw(PainterInfo &, int, int) const
{}


void MathKernInset::write(WriteStream & os) const
{
#ifdef WITH_WARNINGS
#warning this crashs on startup!
#endif
	//os << "\\kern" << wid_.asLatexString() << ' ';
	os << "\\kern0mm ";
}


void MathKernInset::normalize(NormalStream & os) const
{
	os << "[kern " << wid_.asLatexString() << ']';
}
