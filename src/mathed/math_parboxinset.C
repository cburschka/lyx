
#include "math_parboxinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "lyxlength.h"
#include "debug.h"


MathParboxInset::MathParboxInset()
	: lyx_width_(0), tex_width_("0mm"), position_('c')
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


void MathParboxInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	FontSetChanger dummy1(mi.base, "textnormal");
	WidthChanger dummy2(mi.base, lyx_width_);
	MathTextInset::metrics(mi, dim_);
	metricsMarkers();
	dim = dim_;
}


void MathParboxInset::draw(PainterInfo & pi, int x, int y) const
{
	FontSetChanger dummy(pi.base, "textnormal");
	MathTextInset::draw(pi, x + 1, y);
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
	os << "Box: Parbox " << tex_width_;
}
