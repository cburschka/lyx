#ifdef __GNUG__
#pragma implementation
#endif

#include "math_splitinset.h"
#include "support/LOstream.h"


MathSplitInset::MathSplitInset(int n)
	: MathGridInset(2, n)
{
	setDefaults();
}


MathInset * MathSplitInset::clone() const
{
	return new MathSplitInset(*this);
}


void MathSplitInset::write(std::ostream & os, bool fragile) const
{
	if (fragile)
		os << "\\protect";
	os << "\\begin{split}";
	MathGridInset::write(os, fragile);
	if (fragile)
		os << "\\protect";
	os << "\\end{split}\n";
}
