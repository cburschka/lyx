#include <config.h>

#include "math_dotsinset.h"
#include "math_mathmlstream.h"
#include "math_streamstr.h"
#include "math_support.h"
#include "math_parser.h"


MathDotsInset::MathDotsInset(latexkeys const * key)
	: key_(key)
{}


InsetBase * MathDotsInset::clone() const
{
	return new MathDotsInset(*this);
}


void MathDotsInset::metrics(MetricsInfo & mi, Dimension & dim) const
{
	mathed_char_dim(mi.base.font, 'M', dim_);
	dh_ = 0;
	if (key_->name == "cdots" || key_->name == "dotsb"
			|| key_->name == "dotsm" || key_->name == "dotsi")
		dh_ = dim_.asc / 2;
	else if (key_->name == "dotsc")
		dh_ = dim_.asc / 4;
	else if (key_->name == "vdots") {
		dim_.wid = (dim_.wid / 2) + 1;
		dh_ = dim_.asc;
	}
	else if (key_->name == "ddots")
		dh_ = dim_.asc;
	dim = dim_;
}


void MathDotsInset::draw(PainterInfo & pain, int x, int y) const
{
	mathed_draw_deco(pain, x + 2, y - dh_, dim_.width() - 2, dim_.ascent(),
		key_->name);
	if (key_->name == "vdots" || key_->name == "ddots")
		++x;
	if (key_->name != "vdots")
		--y;
	mathed_draw_deco(pain, x + 2, y - dh_, dim_.width() - 2, dim_.ascent(),
		key_->name);
}


string MathDotsInset::name() const
{
	return key_->name;
}
