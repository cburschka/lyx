#ifdef __GNUG__
#pragma implementation
#endif

#include "math_arrayinset.h"
#include "support/LOstream.h"


MathArrayInset::MathArrayInset(int m, int n)
	: MathGridInset(m, n)
{}


MathInset * MathArrayInset::clone() const
{
	return new MathArrayInset(*this);
}


void MathArrayInset::write(std::ostream & os, bool fragile) const
{
	if (fragile)
		os << "\\protect";
	os << "\\begin{array}";

	if (v_align_ == 't' || v_align_ == 'b') 
		os << '[' << char(v_align_) << ']';

	os << '{';
	for (unsigned int col = 0; col < ncols(); ++col)
		os << colinfo_[col].align_;
	os << "}\n";

	MathGridInset::write(os, fragile);

	if (fragile)
		os << "\\protect";
	os << "\\end{array}\n";
}


void MathArrayInset::metrics(MathStyles st) const
{
	MathGridInset::metrics(st == LM_ST_DISPLAY ? LM_ST_TEXT : st);
}

