#include <iostream>
#include "textpainter.h"


TextPainter::TextPainter(int xmax, int ymax)
	: xmax_(xmax), ymax_(ymax), data_(xmax_ * (ymax_ + 1), ' ')
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
	//cerr << "drawing string '" << str << "' at " << x << "," << y << "\n";
	for (int i = 0; *str && x + i < xmax_; ++i, ++str)
		at(x + i, y) = *str;
	//show();
}


void TextPainter::horizontalLine(int x, int y, int n, char c)
{
	for (int i = 0; i < n && i + x < xmax_; ++i)
		at(x + i, y) = c;
}


void TextPainter::verticalLine(int x, int y, int n, char c)
{
	for (int i = 0; i < n && i + y < ymax_; ++i)
		at(x, y + i) = c;
}


void TextPainter::draw(int x, int y, char c)
{
	//cerr << "drawing char '" << c << "' at " << x << "," << y << "\n";
	at(x, y) = c;
	//show();
}


void TextPainter::show(std::ostream & os) const
{
	os << '\n';
	for (int j = 0; j <= ymax_; ++j) {
		for (int i = 0; i < xmax_; ++i)
			os << at(i, j);
		os << '\n';
	}
}
