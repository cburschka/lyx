#include "math_notinset.h"
#include "math_parser.h"
#include "math_mathmlstream.h"
#include "math_support.h"


MathNotInset::MathNotInset()
{}


MathInset * MathNotInset::clone() const
{
	return new MathNotInset(*this);
}


void MathNotInset::write(WriteStream & os) const
{
	os << "\\not ";
}


void MathNotInset::normalize(NormalStream & os) const
{
	os << "[not] ";
}


void MathNotInset::metrics(MathMetricsInfo const & mi) const
{
	if (math_font_available(LM_TC_CMSY)) {
		whichFont(font_, LM_TC_CMSY, mi);
		mathed_char_dim(font_, 54, ascent_, descent_, width_);
	}
	else {
		whichFont(font_, LM_TC_VAR, mi);
		mathed_char_dim(font_, '/', ascent_, descent_, width_);
	}
	width_ = 0;
}


void MathNotInset::draw(Painter & pain, int x, int y) const
{
	if (math_font_available(LM_TC_CMSY))
		drawChar(pain, font_, x, y, 54);
	else
		drawChar(pain, font_, x, y, '/');
}
