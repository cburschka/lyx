#ifdef __GNUG__
#pragma implementation
#endif

#include "math_specialcharinset.h"
#include "math_mathmlstream.h"
#include "math_support.h"


MathSpecialCharInset::MathSpecialCharInset(char c)
	: char_(c)
{}


MathInset * MathSpecialCharInset::clone() const
{
	return new MathSpecialCharInset(*this);
}


void MathSpecialCharInset::metrics(MathMetricsInfo & mi) const
{
	MathShapeChanger dummy(mi.base.font, LyXFont::ITALIC_SHAPE);
	mathed_char_dim(mi.base.font, char_, ascent_, descent_, width_);
}


void MathSpecialCharInset::draw(MathPainterInfo & pi, int x, int y) const
{
	MathShapeChanger dummy(pi.base.font, LyXFont::ITALIC_SHAPE);
	pi.draw(x, y, char_);
}


void MathSpecialCharInset::write(WriteStream & os) const
{
	os << "\\" << char_;
}


void MathSpecialCharInset::normalize(NormalStream & os) const
{
	os << "\\" << char_;
}
