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
	mi_ = mi;
	if (math_font_available(LM_TC_CMSY))
		mathed_char_dim(LM_TC_CMSY, mi_, 54, ascent_, descent_, width_);
	else
		mathed_char_dim(LM_TC_VAR, mi_, '/', ascent_, descent_, width_);
	width_ = 0;
}


void MathNotInset::draw(Painter & pain, int x, int y) const
{  
	if (math_font_available(LM_TC_CMSY))
		drawChar(pain, LM_TC_CMSY, mi_, x, y, 54);
	else
		drawChar(pain, LM_TC_VAR, mi_, x, y, '/');
}
