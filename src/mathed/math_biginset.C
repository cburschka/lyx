#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_biginset.h"
#include "math_support.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"


MathBigInset::MathBigInset(string const & name, string const & delim)
	: name_(name), delim_(delim)
{}


MathInset * MathBigInset::clone() const
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
	return 0.0;
}


void MathBigInset::metrics(MathMetricsInfo & mi) const
{
	double const h = mathed_char_ascent(mi.base.font, 'I');
	double const f = increase();
	dim_.w = 6;
	dim_.a = int(h + f * h);
	dim_.d = int(f * h);
}


void MathBigInset::draw(MathPainterInfo & pi, int x, int y) const
{
	mathed_draw_deco(pi, x + 1, y - ascent(), 4, height(), delim_);
}


void MathBigInset::write(WriteStream & os) const
{
	os << '\\' << name_ << ' ' << delim_;
}


void MathBigInset::normalize(NormalStream & os) const
{
	os << "[" << name_ << ' ' <<  delim_ << ']';
}
