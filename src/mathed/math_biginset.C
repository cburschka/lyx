#include <config.h>

#include "math_biginset.h"
#include "math_support.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathBigInset::MathBigInset(string const & name, string const & delim)
	: name_(name), delim_(delim)
{}


InsetBase * MathBigInset::clone() const
{
	return new MathBigInset(*this);
}


MathBigInset::size_type MathBigInset::size() const
{
	return name_.size() - 4;
}


double MathBigInset::increase() const
{
	switch (size()) {
		case 1:  return 0.2;
		case 2:  return 0.44;
		case 3:  return 0.7;
		default: return 0.0;
	}
}


void MathBigInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	double const h = mathed_char_ascent(mi.base.font, 'I');
	double const f = increase();
	dim_.wid = 6;
	dim_.asc = int(h + f * h);
	dim_.des = int(f * h);
	dim = dim_;
}


void MathBigInset::draw(PainterInfo & pi, int x, int y) const
{
	mathed_draw_deco(pi, x + 1, y - dim_.ascent(), 4, dim_.height(), delim_);
}


void MathBigInset::write(WriteStream & os) const
{
	os << '\\' << name_ << ' ' << delim_;
}


void MathBigInset::normalize(NormalStream & os) const
{
	os << '[' << name_ << ' ' <<  delim_ << ']';
}
