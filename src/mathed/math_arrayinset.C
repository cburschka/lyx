#ifdef __GNUG__
#pragma implementation
#endif

#include "math_arrayinset.h"
#include "support/LOstream.h"


MathArrayInset::MathArrayInset(int m, int n)
	: MathGridInset(m, n)
{}


MathArrayInset::MathArrayInset(int m, int n, char valign, string const & halign)
	: MathGridInset(m, n, valign, halign)
{}


MathInset * MathArrayInset::clone() const
{
	return new MathArrayInset(*this);
}


void MathArrayInset::write(MathWriteInfo & os) const
{
	if (os.fragile)
		os << "\\protect";
	os << "\\begin{array}";

	if (v_align_ == 't' || v_align_ == 'b') 
		os << '[' << char(v_align_) << ']';

	os << '{';
	for (col_type col = 0; col < ncols(); ++col)
		os << colinfo_[col].align_;
	os << "}\n";

	MathGridInset::write(os);

	if (os.fragile)
		os << "\\protect";
	os << "\\end{array}\n";
}


void MathArrayInset::metrics(MathMetricsInfo const & st) const
{
	size_ = st;
	if (size_.style == LM_ST_DISPLAY)
		size_.style = LM_ST_TEXT;
	MathGridInset::metrics(size_);
}

