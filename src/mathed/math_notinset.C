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


void MathNotInset::metrics(MathMetricsInfo & mi) const
{
	font_ = mi.base.font;
	if (math_font_available("cmsy")) {
		augmentFont(font_, "cmsy");
		char_ = 54;
	} else {
		augmentFont(font_, "mathnormal");
		char_ = '/';
	}
	mathed_char_dim(font_, char_, ascent_, descent_, width_);
	width_ = 0;
}


void MathNotInset::draw(MathPainterInfo & pain, int x, int y) const
{
	drawChar(pain, font_, x, y, char_);
}
