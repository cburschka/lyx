#include "textpainter.h"


TextPainter::TextPainter(int xmax, int ymax)
	: xmax_(xmax), ymax_(ymax), data_((xmax_ + 1) * (ymax_ + 1))
{}


char & TextPainter::at(int x, int y)
{
	return data_[y * xmax_ + x];
}


char TextPainter::at(int x, int y) const
{
	return data_[y * xmax_ + x];
}


void TextPainter::draw(int x, int y, char const * str)
{
	for (int i = 0; *str; ++i, ++str)
		at(x + i, y) = *str;
}
