#include "math_notinset.h"
#include "math_parser.h"
#include "support.h"
#include "support/LOstream.h"


using std::ostream;

MathNotInset::MathNotInset()
{}


MathInset * MathNotInset::clone() const
{
	return new MathNotInset(*this);
}


void MathNotInset::write(MathWriteInfo & os) const
{
	os << "\\not ";
}


void MathNotInset::writeNormal(ostream & os) const
{
	os << "[not] ";
}


void MathNotInset::metrics(MathMetricsInfo const & st) const
{
	size_ = st;
	if (math_font_available(LM_TC_CMSY))
		mathed_char_dim(LM_TC_CMSY, size(), 54, ascent_, descent_, width_);
	else
		mathed_char_dim(LM_TC_VAR, size(), '/', ascent_, descent_, width_);
	width_ = 0;
}


void MathNotInset::draw(Painter & pain, int x, int y) const
{  
	xo(x);
	yo(y);

	if (math_font_available(LM_TC_CMSY))
		drawChar(pain, LM_TC_CMSY, size(), x, y, 54);
	else
		drawChar(pain, LM_TC_VAR, size(), x, y, '/');
}
