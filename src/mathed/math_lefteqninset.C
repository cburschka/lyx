#ifdef __GNUG__
#pragma implementation
#endif

#include "math_lefteqninset.h"
#include "LColor.h"
#include "Painter.h"
#include "math_cursor.h"
#include "math_mathmlstream.h"


MathLefteqnInset::MathLefteqnInset()
	: MathNestInset(1)
{}


MathInset * MathLefteqnInset::clone() const
{
	return new MathLefteqnInset(*this);
}


void MathLefteqnInset::metrics(MathMetricsInfo const & mi) const
{
	MathNestInset::metrics(mi);
	ascent_  = xcell(0).ascent() + 2;
	descent_ = xcell(0).descent() + 2;
	width_   = 4;
}


void MathLefteqnInset::draw(Painter & pain, int x, int y) const
{
	xcell(0).draw(pain, x + 2, y);
	if (mathcursor && mathcursor->isInside(this)) {
		pain.rectangle(x, y - ascent(), xcell(0).width(), height(),
			LColor::mathframe);
	}
}


void MathLefteqnInset::write(MathWriteInfo & os) const
{
	os << "\\lefteqn{" << cell(0) << "}";
}


void MathLefteqnInset::writeNormal(NormalStream & os) const
{
	os << "[lefteqn ";
	MathWriteInfo wi(os.os_);
	cell(0).write(wi);
	os << "]";
}
