#include <config.h>

#include "math_ertinset.h"
#include "math_mathmlstream.h"

#include "support/LOstream.h"

MathInset * MathErtInset::clone() const
{
	return new MathErtInset(*this);
}


void MathErtInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "lyxert");
	MathTextInset::metrics(mi);
	cache_.colinfo_[0].align_ = 'l';
	metricsMarkers2();
}


void MathErtInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "lyxert");
	MathTextInset::draw(pi, x + 1, y);
	drawMarkers2(pi, x, y);
}


void MathErtInset::write(WriteStream & os) const
{
	if (os.latex())
		os << cell(0);
	else
		os << "\\lyxert{" << cell(0) << '}';
}


void MathErtInset::infoize(std::ostream & os) const
{
	os << "Box: Ert";
}

