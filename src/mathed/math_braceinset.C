#include <config.h>

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


void MathBraceInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	cell(0).metrics(mi);
	Dimension t;
	mathed_char_dim(mi.base.font, '{', t);
	wid_ = t.wid;
	dim_.asc = max(cell(0).ascent(), t.asc);
	dim_.des = max(cell(0).descent(), t.des);
	dim_.wid = cell(0).width() + 2 * wid_;
	dim = dim_;
}


void MathBraceInset::draw(PainterInfo & pi, int x, int y) const
{
	LyXFont font = pi.base.font;
	font.setColor(LColor::latex);
	drawChar(pi, font, x, y, '{');
	cell(0).draw(pi, x + wid_, y);
	drawChar(pi, font, x + dim_.width() - wid_, y, '}');
}


void MathBraceInset::write(WriteStream & os) const
{
	os << '{' << cell(0) << '}';
}


void MathBraceInset::normalize(NormalStream & os) const
{
	os << "[block " << cell(0) << ']';
}


void MathBraceInset::maple(MapleStream & os) const
{
	os << cell(0);
}


void MathBraceInset::octave(OctaveStream & os) const
{
	os << cell(0);
}


void MathBraceInset::mathmlize(MathMLStream & os) const
{
	os << MTag("mrow") << cell(0) << ETag("mrow");
}


void MathBraceInset::mathematica(MathematicaStream & os) const
{
	os << cell(0);
}


void MathBraceInset::infoize(std::ostream & os) const
{
	os << "Nested Block: ";
}
