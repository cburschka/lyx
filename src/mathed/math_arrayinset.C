#ifdef __GNUG__
#pragma implementation
#endif

#include "math_arrayinset.h"
#include "support/LOstream.h"


MathArrayInset::MathArrayInset(int m, int n)
	: MathGridInset(m, n, "array", LM_OT_MATRIX)
{}


MathInset * MathArrayInset::clone() const
{
	return new MathArrayInset(*this);
}


void MathArrayInset::Write(std::ostream & os, bool fragile) const
{
	if (fragile)
		os << "\\protect";
	os << "\\begin{array}";

	if (v_align_ == 't' || v_align_ == 'b') 
		os << '[' << char(v_align_) << ']';

	os << '{';
	for (int col = 0; col < ncols(); ++col)
		os << colinfo_[col].h_align_;
	os << "}\n";

	MathGridInset::Write(os, fragile);

	if (fragile)
		os << "\\protect";
	os << "\\end{array}\n";
}
