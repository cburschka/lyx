#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "math_dotsinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"


MathDotsInset::MathDotsInset(latexkeys const * key)
	: key_(key)
{}


MathInset * MathDotsInset::clone() const
{
	return new MathDotsInset(*this);
}


void MathDotsInset::metrics(MathMetricsInfo & mi) const
{
	mathed_char_dim(mi.base.font, 'M', dim_);
	if (key_->name == "ldots" || key_->name == "dotsm") 
		dh_ = 0;
	else if (key_->name == "cdots" || key_->name == "dotsb"
			|| key_->name == "dotsm" || key_->name == "dotsi")
		dh_ = ascent() / 2;
	else if (key_->name == "dotsc")
		dh_ = ascent() / 4;
	else if (key_->name == "vdots")
		dim_.w /= 2;
	else if (key_->name == "ddots")
		dh_ = ascent();
}


void MathDotsInset::draw(MathPainterInfo & pain, int x, int y) const
{
	mathed_draw_deco(pain, x + 2, y - dh_, width() - 2, ascent(), key_->name);
	if (key_->name == "vdots" || key_->name == "ddots")
		++x;
	if (key_->name != "vdots")
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, width() - 2, ascent(), key_->name);
}


string MathDotsInset::name() const
{
	return key_->name;
}
