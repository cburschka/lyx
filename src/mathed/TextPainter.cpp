/**
 * \file TextPainter.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "TextPainter.h"


namespace lyx {


TextPainter::TextPainter(int xmax, int ymax)
	: xmax_(xmax), ymax_(ymax), data_(xmax_ * (ymax_ + 1), ' ')
{}


char_type & TextPainter::at(int x, int y)
{
	return data_[y * xmax_ + x];
}


char_type TextPainter::at(int x, int y) const
{
	return data_[y * xmax_ + x];
}


void TextPainter::draw(int x, int y, char_type const * str)
{
	//cerr << "drawing string '" << str << "' at " << x << ',' << y << endl;
	for (int i = 0; *str && x + i < xmax_; ++i, ++str)
		at(x + i, y) = *str;
	//show();
}


void TextPainter::horizontalLine(int x, int y, int n, char_type c)
{
	for (int i = 0; i < n && i + x < xmax_; ++i)
		at(x + i, y) = c;
}


void TextPainter::verticalLine(int x, int y, int n, char_type c)
{
	for (int i = 0; i < n && i + y < ymax_; ++i)
		at(x, y + i) = c;
}


void TextPainter::draw(int x, int y, char_type c)
{
	//cerr << "drawing char '" << c << "' at " << x << ',' << y << endl;
	at(x, y) = c;
	//show();
}


void TextPainter::show(odocstream & os, int offset) const
{
	os << '\n';
	for (int j = 0; j <= ymax_; ++j) {
		for (int i = 0; i < offset; ++i)
			os << ' ';
		for (int i = 0; i < xmax_; ++i)
			os << at(i, j);
		os << '\n';
	}
}


} // namespace lyx
