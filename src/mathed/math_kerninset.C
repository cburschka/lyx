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


void MathKernInset::write(std::ostream & os, bool) const
{
	os << "\\kern" << wid_.asLatexString() << " ";
}


void MathKernInset::writeNormal(std::ostream & os) const
{
	os << "[kern " << wid_.asLatexString() << "]";
}


void MathKernInset::metrics(MathStyles) const
{
	ascent_  = 0;
	descent_ = 0;
#ifdef WITH_WARNINGS
#warning fix this once the interface to LyXLength has improved
#endif
	width_   = static_cast<int>(wid_.value());
}
