#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fboxinset.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "frontends/Painter.h"



MathFboxInset::MathFboxInset()
	: MathNestInset(1)
{}


MathInset * MathFboxInset::clone() const
{
	return new MathFboxInset(*this);
}


void MathFboxInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy(mi.base, "textnormal");
	xcell(0).metrics(mi);
	ascent_  = xcell(0).ascent()  + 5;
	descent_ = xcell(0).descent() + 5;
	width_   = xcell(0).width()   + 10;
}


void MathFboxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy(pi.base, "textnormal");
	pi.pain.rectangle(x + 1, y - ascent() + 1, width() - 2, height() - 2,
			LColor::black);
	xcell(0).draw(pi, x + 5, y);
}


void MathFboxInset::write(WriteStream & os) const
{
	os << "\\fbox{" << cell(0) << '}';
}


void MathFboxInset::normalize(NormalStream & os) const
{
	os << "[fbox " << cell(0) << ']';
}
