#ifdef __GNUG__
#pragma implementation
#endif

#include "math_sqrtinset.h"
#include "math_mathmlstream.h"
#include "LColor.h"
#include "Painter.h"
#include "textpainter.h"


MathSqrtInset::MathSqrtInset()
	: MathNestInset(1)
{}


MathInset * MathSqrtInset::clone() const
{   
	return new MathSqrtInset(*this);
}


void MathSqrtInset::metrics(MathMetricsInfo const & mi) const
{
	xcell(0).metrics(mi);
	ascent_  = xcell(0).ascent()  + 4;
	descent_ = xcell(0).descent() + 2;
	width_   = xcell(0).width()   + 12;
}


void MathSqrtInset::draw(Painter & pain, int x, int y) const
{ 
	xcell(0).draw(pain, x + 10, y); 
	int const a = ascent_;
	int const d = descent_;
	int xp[4];
	int yp[4];
	xp[0] = x + width_; yp[0] = y - a + 1;
	xp[1] = x + 8;      yp[1] = y - a + 1;
	xp[2] = x + 5;      yp[2] = y + d - 1;
	xp[3] = x;          yp[3] = y + (d - a)/2;
	pain.lines(xp, yp, 4, LColor::math);
}


void MathSqrtInset::metricsT(TextMetricsInfo const & mi) const
{
	xcell(0).metricsT(mi);
	ascent_  = xcell(0).ascent()  + 1;
	descent_ = xcell(0).descent();
	width_   = xcell(0).width()   + 2;
}


void MathSqrtInset::drawT(TextPainter & pain, int x, int y) const
{ 
	xcell(0).drawT(pain, x + 2, y); 
	pain.horizontalLine(x + 2, y - xcell(0).ascent(), xcell(0).width(), '_');
	pain.verticalLine  (x + 1, y - xcell(0).ascent() + 1, xcell(0).height());
	pain.draw(x, y + xcell(0).descent(), '\\');
}


void MathSqrtInset::write(WriteStream & os) const
{
	os << "\\sqrt{" << cell(0) << '}';
}


void MathSqrtInset::normalize(NormalStream & os) const
{
	os << "[sqrt " << cell(0) << ']';
}


void MathSqrtInset::maplize(MapleStream & os) const
{
	os << "sqrt(" << cell(0) << ')';
}


void MathSqrtInset::mathmlize(MathMLStream & os) const
{
	os << MTag("msqrt") << cell(0) << ETag("msqrt");
}
