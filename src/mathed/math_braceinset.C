#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_braceinset.h"
#include "math_parser.h"
#include "math_support.h"
#include "math_mathmlstream.h"
#include "support/LOstream.h"

using std::max;


MathBraceInset::MathBraceInset()
	: MathNestInset(1)
{}


MathBraceInset::MathBraceInset(MathArray const & ar)
	: MathNestInset(1)
{
	cell(0) = ar;
}


MathInset * MathBraceInset::clone() const
{
	return new MathBraceInset(*this);
}


void MathBraceInset::metrics(MathMetricsInfo & mi) const
{
	xcell(0).metrics(mi);
	Dimension t;
	mathed_char_dim(mi.base.font, '{', t);
	wid_ = t.w;
	dim_.a = max(xcell(0).ascent(), t.a);
	dim_.d = max(xcell(0).descent(), t.a);
	dim_.w = xcell(0).width() + 2 * wid_;
}


void MathBraceInset::draw(MathPainterInfo & pi, int x, int y) const
{
	LyXFont font = pi.base.font;
	font.setColor(LColor::latex);
	drawChar(pi, font, x, y, '{');
	xcell(0).draw(pi, x + wid_, y);
	drawChar(pi, font, x + width() - wid_, y, '}');
}


void MathBraceInset::write(WriteStream & os) const
{
	os << '{' << cell(0) << '}';
}


void MathBraceInset::normalize(NormalStream & os) const
{
	os << "[block " << cell(0) << ']';
}


void MathBraceInset::infoize(std::ostream & os) const
{
	os << "Nested Block: ";
}
