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


void MathSplitInset::write(MathWriteInfo & os) const
{
	if (os.fragile)
		os << "\\protect";
	os << "\\begin{split}";
	MathGridInset::write(os);
	if (os.fragile)
		os << "\\protect";
	os << "\\end{split}\n";
}
