#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_kerninset.h"
#include "math_extern.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "font.h"


MathKernInset::MathKernInset()
	: MathNestInset(1)
{}


MathInset * MathKernInset::clone() const
{
	return new MathKernInset(*this);
}


void MathKernInset::metrics(MathMetricsInfo const & mi) const
{
	LyXFont font;
	whichFont(font, LM_TC_TEXTRM, mi);
	double t;
	extractNumber(cell(0), t);
	width_   = int(t * lyxfont::width('m', font));
	ascent_  = 0;
	descent_ = 0;
}


void MathKernInset::draw(Painter &, int, int) const
{}


void MathKernInset::write(WriteStream & os) const
{
	os << "\\lyxkern" << cell(0) << "em ";
}


void MathKernInset::normalize(NormalStream & os) const
{
	os << "[lyxkern " << cell(0) << "em]";
}
