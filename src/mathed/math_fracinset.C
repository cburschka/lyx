#ifdef __GNUG__
#pragma implementation
#endif

#include "math_fracinset.h"
#include "math_support.h"
#include "Painter.h"
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


void MathFracInset::metrics(MathMetricsInfo const & mi) const
{
	MathMetricsInfo m = mi;
	smallerStyleFrac(m);
	xcell(0).metrics(m);
	xcell(1).metrics(m);
	width_   = max(xcell(0).width(), xcell(1).width()) + 2;
	ascent_  = xcell(0).height() + 2 + 5;
	descent_ = xcell(1).height() + 2 - 5;
}


void MathFracInset::draw(Painter & pain, int x, int y) const
{
	int m = x + width() / 2;
	xcell(0).draw(pain, m - xcell(0).width() / 2, y - xcell(0).descent() - 2 - 5);
	xcell(1).draw(pain, m - xcell(1).width() / 2, y + xcell(1).ascent()  + 2 - 5);
	if (!atop_)
		pain.line(x, y - 5, x + width(), y - 5, LColor::math);
}


void MathFracInset::metricsT(TextMetricsInfo const & mi) const
{
	xcell(0).metricsT(mi);
	xcell(1).metricsT(mi);
	width_   = max(xcell(0).width(), xcell(1).width());
	ascent_  = xcell(0).height() + 1;
	descent_ = xcell(1).height();
}


void MathFracInset::drawT(TextPainter & pain, int x, int y) const
{
	int m = x + width() / 2;
	xcell(0).drawT(pain, m - xcell(0).width() / 2, y - xcell(0).descent() - 1);
	xcell(1).drawT(pain, m - xcell(1).width() / 2, y + xcell(1).ascent());
	if (!atop_)
		pain.horizontalLine(x, y, width());
}


void MathFracInset::write(WriteStream & os) const
{
	if (atop_)
		os << '{' << cell(0) << "\\atop " << cell(1) << '}';
	else
		os << "\\frac{" << cell(0) << "}{" << cell(1) << '}';
}


void MathFracInset::normalize(NormalStream & os) const
{
	if (atop_)
		os << "[atop ";
	else
		os << "[frac ";
	os << cell(0) << ' ' << cell(1) << ']';
}


void MathFracInset::maplize(MapleStream & os) const
{
	os << '(' << cell(0) << ")/(" << cell(1) << ')';
}


void MathFracInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mfrac") << cell(0) << cell(1) << ETag("mfrac");
}
