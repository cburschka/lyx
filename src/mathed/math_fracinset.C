#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fracinset.h"
#include "math_support.h"
#include "frontends/Painter.h"
#include "math_mathmlstream.h"
#include "textpainter.h"


using std::max;


MathFracInset::MathFracInset(bool atop)
	: atop_(atop)
{}


MathInset * MathFracInset::clone() const
{
	return new MathFracInset(*this);
}


MathFracInset * MathFracInset::asFracInset()
{
	return atop_ ? 0 : this;
}


MathFracInset const * MathFracInset::asFracInset() const
{
	return atop_ ? 0 : this;
}


void MathFracInset::metrics(MathMetricsInfo & mi) const
{
	MathFracChanger dummy(mi.base);
	cell(0).metrics(mi);
	cell(1).metrics(mi);
	dim_.w = max(cell(0).width(), cell(1).width()) + 2;
	dim_.a = cell(0).height() + 2 + 5;
	dim_.d = cell(1).height() + 2 - 5;
}


void MathFracInset::draw(MathPainterInfo & pi, int x, int y) const
{
	int m = x + width() / 2;
	MathFracChanger dummy(pi.base);
	cell(0).draw(pi, m - cell(0).width() / 2, y - cell(0).descent() - 2 - 5);
	cell(1).draw(pi, m - cell(1).width() / 2, y + cell(1).ascent()  + 2 - 5);
	if (!atop_)
		pi.pain.line(x + 1, y - 5, x + width() - 2, y - 5, LColor::math);
}


void MathFracInset::metricsT(TextMetricsInfo const & mi) const
{
	cell(0).metricsT(mi);
	cell(1).metricsT(mi);
	dim_.w = max(cell(0).width(), cell(1).width());
	dim_.a = cell(0).height() + 1;
	dim_.d = cell(1).height();
}


void MathFracInset::drawT(TextPainter & pain, int x, int y) const
{
	int m = x + width() / 2;
	cell(0).drawT(pain, m - cell(0).width() / 2, y - cell(0).descent() - 1);
	cell(1).drawT(pain, m - cell(1).width() / 2, y + cell(1).ascent());
	if (!atop_)
		pain.horizontalLine(x, y, width());
}


void MathFracInset::write(WriteStream & os) const
{
	if (atop_)
		os << '{' << cell(0) << "\\atop " << cell(1) << '}';
	else // it's \\frac
		MathNestInset::write(os);
}


string MathFracInset::name() const
{
	return atop_ ? "atop" : "frac";
}


void MathFracInset::maplize(MapleStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void MathFracInset::mathematicize(MathematicaStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void MathFracInset::octavize(OctaveStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void MathFracInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mfrac") << cell(0) << cell(1) << ETag("mfrac");
}
