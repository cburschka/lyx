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


void MathDotsInset::metrics(MathMetricsInfo const & mi) const
{
	LyXFont font;
	whichFont(font, LM_TC_VAR, mi);
	mathed_char_dim(font, 'M', ascent_, descent_, width_);
	if (name_ == "ldots" || name_ == "dotsm") 
		dh_ = 0;
	else if (name_ == "cdots" || name_ == "dotsb"
			|| name_ == "dotsm" || name_ == "dotsi")
		dh_ = ascent_ / 2;
	else if (name_ == "dotsc")
		dh_ = ascent_ / 4;
	else if (name_ == "vdots")
		width_ /= 2;
	else if (name_ == "ddots")
		dh_ = ascent_;
}


void MathDotsInset::draw(Painter & pain, int x, int y) const
{
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, name_);
	if (name_ == "vdots" || name_ == "ddots")
		++x;
	if (name_ != "vdots")
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, width_ - 2, ascent_, name_);
}


void MathDotsInset::write(WriteStream & os) const
{
	os << '\\' << name_ << ' ';
}


void MathDotsInset::normalize(NormalStream & os) const
{
	os << "[" << name_ << "] ";
}
