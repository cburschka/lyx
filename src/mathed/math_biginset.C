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


void MathBigInset::metrics(MathMetricsInfo const & mi) const
{
	LyXFont font;
	whichFont(font, LM_TC_VAR, mi);
	double h = mathed_char_ascent(font, 'I');
	double f = increase();
	width_   = 6;
	ascent_  = int(h + f * h);
	descent_ = int(f * h);
}


void MathBigInset::draw(Painter & pain, int x, int y) const
{
	mathed_draw_deco(pain, x + 1, y - ascent_, 4, height(), delim_);
}


void MathBigInset::write(WriteStream & os) const
{
	os << '\\' << name_ << ' ' << delim_;
}


void MathBigInset::normalize(NormalStream & os) const
{
	os << "[" << name_ << ' ' <<  delim_ << ']';
}
