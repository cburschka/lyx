#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_binominset.h"
#include "math_support.h"
#include "support/LOstream.h"
#include "math_mathmlstream.h"


using std::max;


MathBinomInset::MathBinomInset(bool choose)
	: choose_(choose)
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


void MathBinomInset::metrics(MathMetricsInfo & mi) const
{
	MathScriptChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.a = cell(0).height() + 4 + 5;
	dim_.d = cell(1).height() + 4 - 5;
	dim_.w = max(cell(0).width(), cell(1).width()) + 2 * dw() + 4;
}


void MathBinomInset::draw(MathPainterInfo & pi, int x, int y) const
{
	int m = x + width() / 2;
	MathScriptChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, y - cell(0).descent() - 3 - 5);
	cell(1).draw(pi, m - cell(1).width() / 2, y + cell(1).ascent()  + 3 - 5);
	mathed_draw_deco(pi, x, y - ascent(), dw(), height(), "(");
	mathed_draw_deco(pi, x + width() - dw(), y - ascent(),	dw(), height(), ")");
}


void MathBinomInset::write(WriteStream & os) const
{
	if (choose_)
		os << '{' << cell(0) << " \\choose " << cell(1) << '}';
	else
		os << "\\binom{" << cell(0) << "}{" << cell(1) << '}';
}


void MathBinomInset::normalize(NormalStream & os) const
{
	os << "[binom " << cell(0) << ' ' << cell(1) << ']';
}
