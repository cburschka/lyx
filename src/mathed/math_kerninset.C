#include <config.h>


#include "math_kerninset.h"
#include "math_extern.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"


MathKernInset::MathKernInset()
{}


MathKernInset::MathKernInset(LyXLength const & w)
	: wid_(w)
{}


MathKernInset::MathKernInset(string const & s)
	: wid_(s)
{}


MathInset * MathKernInset::clone() const
{
	return new MathKernInset(*this);
}


Dimension MathKernInset::metrics(MetricsInfo & mi) const
{
	Dimension dim;
	dim.wid = wid_.inPixels(0, mathed_char_width(mi.base.font, 'M'));
	dim.asc = 0;
	dim.des = 0;
	return dim;
}


void MathKernInset::draw(PainterInfo &, int, int) const
{}


void MathKernInset::write(WriteStream & os) const
{
	os << "\\kern" << wid_.asLatexString() << ' ';
}


void MathKernInset::normalize(NormalStream & os) const
{
	os << "[kern " << wid_.asLatexString() << ']';
}
