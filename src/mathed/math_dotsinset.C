#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_dotsinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"


MathDotsInset::MathDotsInset(string const & name)
	: name_(name)
{}


MathInset * MathDotsInset::clone() const
{
	return new MathDotsInset(*this);
}     


void MathDotsInset::draw(Painter & pain, int x, int y) const
{
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, name_);
	char const c = name_[0];
	if (c == 'v' || c == 'd')
		++x;
	if (c != 'v')
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, name_);
}


void MathDotsInset::metrics(MathMetricsInfo const & mi) const
{
	mathed_char_dim(LM_TC_VAR, mi, 'M', ascent_, descent_, width_);
	switch (name_[0]) {
		case 'l': dh_ = 0; break;
		case 'c': dh_ = ascent_ / 2; break;
		case 'v': width_ /= 2;
		case 'd': dh_ = ascent_; break;
	}
} 


void MathDotsInset::write(WriteStream & os) const
{
	os << '\\' << name_ << ' ';
}


void MathDotsInset::normalize(NormalStream & os) const
{
	os << "[" << name_ << "] ";
}
