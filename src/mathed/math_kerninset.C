#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_kerninset.h"
#include "math_extern.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "lyxrc.h"


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


void MathKernInset::metrics(MathMetricsInfo & /*mi*/) const
{
	dim_.w = wid_.inBP();
	dim_.a = 0;
	dim_.d = 0;
}


void MathKernInset::draw(MathPainterInfo &, int, int) const
{}


void MathKernInset::write(WriteStream & os) const
{
	os << "\\kern" << wid_.asLatexString() << " ";
}


void MathKernInset::normalize(NormalStream & os) const
{
	os << "[kern " << wid_.asLatexString() << "]";
}
