#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_numberinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "debug.h"


MathNumberInset::MathNumberInset(string const & s)
	: str_(s)
{}


MathInset * MathNumberInset::clone() const
{
	return new MathNumberInset(*this);
}


void MathNumberInset::metrics(MathMetricsInfo & mi) const
{
	mathed_string_dim(mi.base.font, str_, ascent_, descent_, width_);
}


void MathNumberInset::draw(MathPainterInfo & pi, int x, int y) const
{
	//lyxerr << "drawing '" << str_ << "' code: " << code_ << endl;
	drawStr(pi, pi.base.font, x, y, str_);
}


void MathNumberInset::normalize(NormalStream & os) const
{
	os << "[number " << str_ << "]";
}


void MathNumberInset::maplize(MapleStream & os) const
{
	os << str_;
}


void MathNumberInset::octavize(OctaveStream & os) const
{
	os << str_;
}


void MathNumberInset::mathmlize(MathMLStream & os) const
{
	os << "<mi> " << str_ << " </mi>";
}


void MathNumberInset::write(WriteStream & os) const
{
	os << str_;
}
