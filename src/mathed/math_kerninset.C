#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_kerninset.h"
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


void MathKernInset::draw(Painter &, int, int) const
{}


void MathKernInset::metrics(MathMetricsInfo const &) const
{
	ascent_  = 0;
	descent_ = 0;
#ifdef WITH_WARNINGS
#warning fix this once the interface to LyXLength has improved
#endif
	// this uses the numerical valu in pixels, even if the unit is cm or ex!
	width_   = static_cast<int>(wid_.value());
	width_   = (width_*static_cast<int>(lyxrc.zoom))/150;
	//cerr << "handling kern of width " << wid_.value() << "\n";
}


void MathKernInset::write(WriteStream & os) const
{
	os << "\\kern" << wid_.asLatexString() << " ";
}


void MathKernInset::normalize(NormalStream & os) const
{
	os << "[kern " << wid_.asLatexString() << "]";
}

