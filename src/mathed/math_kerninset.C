#ifdef __GNUG__
#pragma implementation
#endif

#include "math_kerninset.h"
#include "support.h"
#include "support/LOstream.h"


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


void MathKernInset::write(MathWriteInfo & os) const
{
	os << "\\kern" << wid_.asLatexString() << " ";
}


void MathKernInset::writeNormal(std::ostream & os) const
{
	os << "[kern " << wid_.asLatexString() << "]";
}


void MathKernInset::metrics(MathMetricsInfo const &) const
{
	ascent_  = 0;
	descent_ = 0;
#ifdef WITH_WARNINGS
#warning fix this once the interface to LyXLength has improved
#endif
	// this uses the numerical valu in pixels, even if the unit is cm or ex!
	width_   = static_cast<int>(wid_.value());
	//cerr << "handling kern of width " << wid_.value() << "\n";
}
