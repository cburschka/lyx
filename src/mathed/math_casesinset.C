#ifdef __GNUG__
#pragma implementation
#endif

#include "math_casesinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_support.h"
#include "Painter.h"


MathCasesInset::MathCasesInset(row_type n)
	: MathGridInset(2, n, 'c', "ll")
{}


MathInset * MathCasesInset::clone() const
{
	return new MathCasesInset(*this);
}


void MathCasesInset::metrics(MathMetricsInfo const & mi) const
{
	MathGridInset::metrics(mi);
	width_ += 8;
}


void MathCasesInset::draw(Painter & pain, int x, int y) const
{ 
	mathed_draw_deco(pain, x + 1, y - ascent(), 6, height(), "{");
	MathGridInset::draw(pain, x + 8, y);
}


void MathCasesInset::write(WriteStream & os) const
{
	if (os.fragile)
		os << "\\protect";
	os << "\\begin{cases}";
	MathGridInset::write(os);
	if (os.fragile)
		os << "\\protect";
	os << "\\end{cases}\n";
}


void MathCasesInset::normalize(NormalStream & os) const
{
	os << "[cases ";
	MathGridInset::normalize(os);
	os << "]";
}


void MathCasesInset::maplize(MapleStream & os) const
{
	os << "cases(";
	MathGridInset::maplize(os);
	os << ")";
}
