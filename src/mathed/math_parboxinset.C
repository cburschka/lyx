
#include "math_parboxinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "lyxlength.h"
#include "debug.h"


MathParboxInset::MathParboxInset()
	: MathNestInset(1), lyx_width_(0), tex_width_("0mm"),
	  position_('c')
{
	lyxerr << "constructing MathParboxInset\n";
}


MathInset * MathParboxInset::clone() const
{
	return new MathParboxInset(*this);
}


void MathParboxInset::setPosition(string const & p)
{
	position_ = p.size() > 0 ? p[0] : 'c';
}


void MathParboxInset::setWidth(string const & w)
{
	tex_width_ = w;
	lyx_width_ = LyXLength(w).inBP();
	lyxerr << "setting " << w << " to " << lyx_width_ << " pixel\n";
}


void MathParboxInset::metrics(MathMetricsInfo & mi) const
{
	MathFontSetChanger dummy1(mi.base, "textnormal");
	MathWidthChanger   dummy2(mi.base, lyx_width_);
	xcell(0).metrics(mi);
	ascent_  = xcell(0).ascent();
	descent_ = xcell(0).descent() + 1;
	width_   = xcell(0).width()   + 2;
}


void MathParboxInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathFontSetChanger dummy1(pi.base, "textnormal");
	MathWidthChanger   dummy2(pi.base, lyx_width_);
	xcell(0).draw(pi, x + 1, y);
	drawMarkers(pi, x, y);
}


void MathParboxInset::write(WriteStream & os) const
{
	os << "\\parbox";
	if (position_ != 'c')
		os << '[' << position_ << ']';
	os << '{' << tex_width_ << "}{" << cell(0) << '}';
}


void MathParboxInset::infoize(std::ostream & os) const
{
	os << "Box: Parbox " << tex_width_ << ' ';
}

