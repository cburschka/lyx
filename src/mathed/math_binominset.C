#ifdef __GNUG__
#pragma implementation
#endif

#include "math_binominset.h"
#include "math_support.h"
#include "support/LOstream.h"
#include "math_mathmlstream.h"


MathBinomInset::MathBinomInset()
{}


MathInset * MathBinomInset::clone() const
{   
	return new MathBinomInset(*this);
}


int MathBinomInset::dw() const
{
	int w = height()/5;
	if (w > 15)
		w = 15;
	if (w < 6)
		w = 6;
	return w;
}


void MathBinomInset::metrics(MathMetricsInfo const & st) const
{
	MathMetricsInfo mi = st;
	smallerStyleFrac(mi);
	xcell(0).metrics(mi);
	xcell(1).metrics(mi);
	ascent_  = xcell(0).height() + 4 + 5;
	descent_ = xcell(1).height() + 4 - 5; 
	width_   = std::max(xcell(0).width(), xcell(1).width()) + 2 * dw() + 4; 
}


void MathBinomInset::draw(Painter & pain, int x, int y) const
{
	int m = x + width() / 2;
	xcell(0).draw(pain, m - xcell(0).width() / 2, y - xcell(0).descent() - 3 - 5);
	xcell(1).draw(pain, m - xcell(1).width() / 2, y + xcell(1).ascent()  + 3 - 5);
	mathed_draw_deco(pain, x, y - ascent_, dw(), height(), "(");
	mathed_draw_deco(pain, x + width() - dw(), y - ascent_,	dw(), height(), ")");
}


void MathBinomInset::write(MathWriteInfo & os) const
{
	os << '{' << cell(0) << " \\choose " << cell(1) << '}';
}


void MathBinomInset::writeNormal(NormalStream & os) const
{
	os << "[binom ";
	cell(0).writeNormal(os);
	os << " ";
	cell(1).writeNormal(os);
	os << "] ";
}
